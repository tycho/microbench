#include <stdint.h>
#include <time.h>

void libtime_init(void);
uint64_t libtime_wall(void);
static inline uint64_t libtime_cpu(void)
{
    uint32_t lo, hi;

    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t) hi << 32ULL) | lo;
}
uint64_t libtime_cpu_to_wall(uint64_t clock);
