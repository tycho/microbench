#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <x86intrin.h>
#include <sys/mman.h>

#include "libtime.h"

#define TRIALS 1

/* For Intel compiler */
#ifndef __POPCNT__
#ifdef __SSE4_2__
#define __POPCNT__ 1
#endif
#endif

//#define SINGLE

#ifdef SINGLE
#define LINKAGE
#define POPCNT_VEC1(func)
#define POPCNT_VEC2(func)
#define POPCNT_VEC4(func)
#else
#define LINKAGE static inline
#define VEC_LINKAGE __attribute__((noinline))
#define POPCNT_VEC1(func) \
	VEC_LINKAGE uint64_t vec_ ##func(uint64_t *p, uint64_t n) \
	{ \
		uint64_t c = 0; \
		for (uint64_t i = 0; i < n; i++) { \
			c += func(p[i + 0]); \
		} \
		return c; \
	}
#define POPCNT_VEC2(func) \
	VEC_LINKAGE uint64_t vec_ ##func(uint64_t *p, uint64_t n) \
	{ \
		uint64_t c = 0; \
		for (uint64_t i = 0; i < n; i += 2) { \
			c += func(p[i + 0]); \
			c += func(p[i + 1]); \
		} \
		return c; \
	}
#define POPCNT_VEC4(func) \
	VEC_LINKAGE uint64_t vec_ ##func(uint64_t *p, uint64_t n) \
	{ \
		uint64_t c = 0; \
		for (uint64_t i = 0; i < n; i += 4) { \
			c += func(p[i + 0]); \
			c += func(p[i + 1]); \
			c += func(p[i + 2]); \
			c += func(p[i + 3]); \
		} \
		return c; \
	}
#define POPCNT_VEC8(func) \
	VEC_LINKAGE uint64_t vec_ ##func(uint64_t *p, uint64_t n) \
	{ \
		uint64_t c = 0; \
		for (uint64_t i = 0; i < n; i += 8) { \
			c += func(p[i + 0]); \
			c += func(p[i + 1]); \
			c += func(p[i + 2]); \
			c += func(p[i + 3]); \
			c += func(p[i + 4]); \
			c += func(p[i + 5]); \
			c += func(p[i + 6]); \
			c += func(p[i + 7]); \
		} \
		return c; \
	}
#endif

#define POPCNT_VEC POPCNT_VEC4


#ifdef __POPCNT__
LINKAGE uint64_t popcnt_asm(uint64_t n)
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
POPCNT_VEC(popcnt_asm);

LINKAGE uint64_t popcnt_intrin(uint64_t n)
{
	return _mm_popcnt_u64(n);
}
POPCNT_VEC(popcnt_intrin);
#endif

LINKAGE uint64_t popcnt_builtin(uint64_t n)
{
	return __builtin_popcountll(n);
}
POPCNT_VEC(popcnt_builtin);

LINKAGE uint64_t popcnt_c1(uint64_t n)
{
	uint64_t r = 0;
	while(n) {
		r++;
		n &= (n - 1);
	}
	return r;
}
POPCNT_VEC(popcnt_c1);

#if 0
LINKAGE uint64_t popcnt_c2(uint64_t n)
{
	uint64_t r = 0;
	while(n) {
		if (n & 1)
			r++;
		n >>= 1;
	}
	return r;
}
POPCNT_VEC(popcnt_c2);

LINKAGE uint64_t popcnt_c3(uint64_t n)
{
	uint64_t r = 0;
	while(n) {
		r += (n & 1);
		n >>= 1;
	}
	return r;
}
POPCNT_VEC(popcnt_c3);
#endif

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
			count += vec_popcnt_builtin(buffer, elems);
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
			count += vec_popcnt_intrin(buffer, elems);
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
			count += vec_popcnt_asm(buffer, elems);
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
			count += vec_popcnt_c1(buffer, elems);
		}
		endP = libtime_cpu();
		duration = (double)libtime_cpu_to_wall(endP - startP) * 1e-9;
		printf("plain c\t\t%" PRIu64 "\t%6.3lf sec\t%6.3lf GiB/sec\n",
			count, duration, (TRIALS * size / 1024.0 / 1024.0 / 1024.0) / duration);
	}

	munmap(buffer, size);
}
