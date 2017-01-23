#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <sys/mman.h>

#include "libtime.h"

#define TRIALS 100

/* For Intel compiler */
#ifndef __POPCNT__
#ifdef __SSE4_2__
#define __POPCNT__ 1
#endif
#endif

#ifdef __POPCNT__
static inline uint64_t popcnt_asm(uint64_t n)
{
	uint64_t r;
	asm("xorq    %q0, %q0\n\t"
	    "popcntq %q1, %q0\n\t"
	    : "=r" (r)
	    : "rm" (n)
	    : "cc"
	);
	return r;
}
#endif

static inline uint64_t popcnt_c(uint64_t n)
{
	uint64_t r = 0;
	while(n) {
		r++;
		n &= (n - 1);
	}
	return r;
}

int main(int argc, char* argv[])
{
	uint64_t size, elems;
	uint64_t *buffer;
	uint64_t startP, endP;
	uint64_t count;
	double duration;

	if (argc != 2) {
	   fprintf(stderr, "usage: array_size in MB\n");
	   return -1;
	}

	/* MiB -> B */
	size = atol(argv[1]) << 20;
	elems = size / sizeof(uint64_t);

	buffer = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, (off_t)0);
	if (!buffer) {
		fprintf(stderr, "malloc failed\n");
		return -2;
	}
	madvise(buffer, size, MADV_HUGEPAGE);

	libtime_init();

	for (unsigned i = 0; i < elems; ++i) {
		uint32_t *v = (uint32_t *)&buffer[i];
		v[0] = random();
		v[1] = random();
	}

	{
		startP = libtime_cpu();
		count = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with unsigned
			for (uint64_t i = 0; i < elems; i += 4) {
				count += __builtin_popcountll(buffer[i]);
				count += __builtin_popcountll(buffer[i+1]);
				count += __builtin_popcountll(buffer[i+2]);
				count += __builtin_popcountll(buffer[i+3]);
			}
		}
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("builtin\t\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

#ifdef __POPCNT__
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
		printf("intrinsic\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	{
		startP = libtime_cpu();
		count = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				count += popcnt_asm(buffer[i]);
				count += popcnt_asm(buffer[i+1]);
				count += popcnt_asm(buffer[i+2]);
				count += popcnt_asm(buffer[i+3]);
			}
		}
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("asm\t\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}
#endif

	{
		startP = libtime_cpu();
		count = 0;
		for (unsigned k = 0; k < TRIALS; k++) {
			// Tight unrolled loop with uint64_t
			for (uint64_t i = 0; i < elems; i += 4) {
				count += popcnt_c(buffer[i]);
				count += popcnt_c(buffer[i+1]);
				count += popcnt_c(buffer[i+2]);
				count += popcnt_c(buffer[i+3]);
			}
		}
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("plain c\t\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	munmap(buffer, size);
}
