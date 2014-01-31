#include <stdint.h>

#ifndef __included_libtime_h
#define __included_libtime_h

#ifdef _MSC_VER
#  if _MSC_VER > 1200
#    include <intrin.h>
#    pragma intrinsic(__rdtsc)
#  else
#    include <windows.h>
#  endif
#  define inline __inline
#endif

void libtime_init(void);

uint64_t libtime_wall(void);

static inline uint64_t libtime_cpu(void)
{
#ifdef _MSC_VER
#  if _MSC_VER > 1200
    return __rdtsc();
#  else
	LARGE_INTEGER ticks;
	__asm {
		rdtsc
		mov ticks.HighPart, edx
		mov ticks.LowPart, eax
	}
	return ticks.QuadPart;
#  endif
#else
    uint32_t lo, hi;

    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t) hi << 32ULL) | lo;
#endif
}
uint64_t libtime_cpu_to_wall(uint64_t clock);

void libtime_nanosleep(int64_t ns);

#endif
