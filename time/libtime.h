#include <stdint.h>

#ifndef __included_libtime_h
#define __included_libtime_h

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#endif

void libtime_init(void);

uint64_t libtime_wall(void);
static inline uint64_t libtime_cpu(void)
{
#ifdef _MSC_VER
    return __rdtsc();
#else
    uint32_t lo, hi;

    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t) hi << 32ULL) | lo;
#endif
}
uint64_t libtime_cpu_to_wall(uint64_t clock);

void libtime_nanosleep(int64_t ns);

#endif
