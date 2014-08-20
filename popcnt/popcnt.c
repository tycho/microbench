#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <x86intrin.h>
#include <sys/mman.h>
#include <unistd.h>

#include "libtime.h"

#define TRIALS 100

int main(int argc, char* argv[])
{
	uint64_t size, elems;
	uint64_t *buffer;
	uint64_t startP, endP;
	uint64_t count, c0, c1, c2, c3;
	double duration;

	if (argc != 2) {
	   fprintf(stderr, "usage: array_size in MB\n");
	   return -1;
	}

	/* MiB -> B */
	size = atol(argv[1]) << 20;
	//size <<= 20;
	elems = size / sizeof(uint64_t);

	//buffer = malloc(size);
	buffer = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, (off_t)0);
	if (!buffer) {
		fprintf(stderr, "malloc failed\n");
		return -2;
	}
	madvise(buffer, size, MADV_HUGEPAGE);

	libtime_init();

	printf("Prefaulting pages... ");
	fflush(stdout);
	memset(buffer, 0, size);
	printf("DONE\n");

	{
		startP = libtime_cpu();
//#pragma omp parallel for schedule(static, 1024 * 1024)
		for (unsigned i = 0; i < elems; ++i) {
#ifdef __RDRND__
			int z = 0;
			do {
				z = _rdrand64_step((unsigned long long *)&buffer[i]);
				if (!z)
					usleep(1000 * (rand() % 13));
			} while(!z);
#else
			uint32_t *v = &buffer[i];
			v[0] = random();
			v[1] = random();
#endif
		}
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("rdrand fill\t%6.3lf sec\t%6.3lf MiB/sec\n",
			duration, (size / 1024.0 / 1024.0) / duration);
	}

	{
		startP = libtime_cpu();
		count = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with unsigned
			for (unsigned i = 0; i < elems; i += 4) {
				count += _mm_popcnt_u64(buffer[i]);
				count += _mm_popcnt_u64(buffer[i+1]);
				count += _mm_popcnt_u64(buffer[i+2]);
				count += _mm_popcnt_u64(buffer[i+3]);
			}
		}
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("unsigned\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	{
		startP = libtime_cpu();
		count = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				count += _mm_popcnt_u64(buffer[i]);
				count += _mm_popcnt_u64(buffer[i+1]);
				count += _mm_popcnt_u64(buffer[i+2]);
				count += _mm_popcnt_u64(buffer[i+3]);
			}
		}
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("uint64_t\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	{
		startP = libtime_cpu();
		count = 0;
		c0 = c1 = c2 = c3 = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				uint64_t r0 = buffer[i + 0];
				uint64_t r1 = buffer[i + 1];
				uint64_t r2 = buffer[i + 2];
				uint64_t r3 = buffer[i + 3];
				__asm__(
					"popcnt %4, %4  \n\t"
					"add %4, %0     \n\t"
					"popcnt %5, %5  \n\t"
					"add %5, %1     \n\t"
					"popcnt %6, %6  \n\t"
					"add %6, %2     \n\t"
					"popcnt %7, %7  \n\t"
					"add %7, %3     \n\t"
					: "+r" (c0), "+r" (c1), "+r" (c2), "+r" (c3)
					: "r"  (r0), "r"  (r1), "r"  (r2), "r"  (r3)
				);
			}
		}
		count = c0 + c1 + c2 + c3;
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("parallel chn\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	{
		startP = libtime_cpu();
		count = 0;
		c0 = c1 = c2 = c3 = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				uint64_t r0 = buffer[i + 0];
				uint64_t r1 = buffer[i + 1];
				uint64_t r2 = buffer[i + 2];
				uint64_t r3 = buffer[i + 3];
				__asm__(
					"popcnt %4, %%rax   \n\t"
					"add %%rax, %0      \n\t"
					"popcnt %5, %5   \n\t"
					"add %5, %1      \n\t"
					"popcnt %6, %6   \n\t"
					"add %6, %2      \n\t"
					"popcnt %7, %7   \n\t"
					"add %7, %3      \n\t"
					: "+r" (c0), "+r" (c1), "+r" (c2), "+r" (c3)
					: "r"  (r0), "r"  (r1), "r"  (r2), "r"  (r3)
					: "rax"
				);
			}
		}
		count = c0 + c1 + c2 + c3;
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("chaining 1\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}


	{
		startP = libtime_cpu();
		count = 0;
		c0 = c1 = c2 = c3 = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				uint64_t r0 = buffer[i + 0];
				uint64_t r1 = buffer[i + 1];
				uint64_t r2 = buffer[i + 2];
				uint64_t r3 = buffer[i + 3];
				__asm__(
					"popcnt %4, %%rax   \n\t"
					"add %%rax, %0      \n\t"
					"popcnt %5, %%rax   \n\t"
					"add %%rax, %1      \n\t"
					"popcnt %6, %6      \n\t"
					"add %6, %2         \n\t"
					"popcnt %7, %7      \n\t"
					"add %7, %3         \n\t"
					: "+r" (c0), "+r" (c1), "+r" (c2), "+r" (c3)
					: "r"  (r0), "r"  (r1), "r"  (r2), "r"  (r3)
					: "rax"
				);
			}
		}
		count = c0 + c1 + c2 + c3;
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("chaining 2\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}


	{
		startP = libtime_cpu();
		count = 0;
		c0 = c1 = c2 = c3 = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				uint64_t r0 = buffer[i + 0];
				uint64_t r1 = buffer[i + 1];
				uint64_t r2 = buffer[i + 2];
				uint64_t r3 = buffer[i + 3];
				__asm__(
					"popcnt %4, %%rax   \n\t"
					"add %%rax, %0      \n\t"
					"popcnt %5, %%rax   \n\t"
					"add %%rax, %1      \n\t"
					"popcnt %6, %%rax   \n\t"
					"add %%rax, %2      \n\t"
					"popcnt %7, %7      \n\t"
					"add %7, %3         \n\t"
					: "+r" (c0), "+r" (c1), "+r" (c2), "+r" (c3)
					: "r"  (r0), "r"  (r1), "r"  (r2), "r"  (r3)
					: "rax"
				);
			}
		}
		count = c0 + c1 + c2 + c3;
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("chaining 3\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}


	{
		startP = libtime_cpu();
		count = 0;
		c0 = c1 = c2 = c3 = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				uint64_t r0 = buffer[i + 0];
				uint64_t r1 = buffer[i + 1];
				uint64_t r2 = buffer[i + 2];
				uint64_t r3 = buffer[i + 3];
				__asm__(
					"popcnt %4, %%rax   \n\t"
					"add %%rax, %0      \n\t"
					"popcnt %5, %%rax   \n\t"
					"add %%rax, %1      \n\t"
					"popcnt %6, %%rax   \n\t"
					"add %%rax, %2      \n\t"
					"popcnt %7, %%rax   \n\t"
					"add %%rax, %3      \n\t"
					: "+r" (c0), "+r" (c1), "+r" (c2), "+r" (c3)
					: "r"  (r0), "r"  (r1), "r"  (r2), "r"  (r3)
					: "rax"
				);
			}
		}
		count = c0 + c1 + c2 + c3;
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("chaining 4\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	{
		startP = libtime_cpu();
		count = 0;
		c0 = c1 = c2 = c3 = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				uint64_t r0 = buffer[i + 0];
				uint64_t r1 = buffer[i + 1];
				uint64_t r2 = buffer[i + 2];
				uint64_t r3 = buffer[i + 3];
				__asm__(
					"xor %%rax, %%rax   \n\t"   // <--- Break the chain.
					"popcnt %4, %%rax   \n\t"
					"add %%rax, %0      \n\t"
					"xor %%rax, %%rax   \n\t"   // <--- Break the chain.
					"popcnt %5, %%rax   \n\t"
					"add %%rax, %1      \n\t"
					"xor %%rax, %%rax   \n\t"   // <--- Break the chain.
					"popcnt %6, %%rax   \n\t"
					"add %%rax, %2      \n\t"
					"xor %%rax, %%rax   \n\t"   // <--- Break the chain.
					"popcnt %7, %%rax   \n\t"
					"add %%rax, %3      \n\t"
					: "+r" (c0), "+r" (c1), "+r" (c2), "+r" (c3)
					: "r"  (r0), "r"  (r1), "r"  (r2), "r"  (r3)
					: "rax"
				);
			}
		}
		count = c0 + c1 + c2 + c3;
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("unchained\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	//free(buffer);
	munmap(buffer, size);
}
