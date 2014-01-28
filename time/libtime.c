#ifdef __MACH__
#include <mach/mach_time.h>
#else
#include <time.h>
#endif
#include <math.h>

#include "libtime.h"

static uint32_t cycles_per_usec;

static uint32_t get_cycles_per_usec(void)
{
    uint64_t wc_s, wc_e;
    uint64_t c_s, c_e;

    wc_s = libtime_wall();
    c_s = libtime_cpu();
    do {
        uint64_t elapsed;

        wc_e = libtime_wall();
        elapsed = wc_e - wc_s;
        if (elapsed >= 1280000ULL) {
            c_e = libtime_cpu();
            break;
        }
    } while (1);

    return (c_e - c_s + 127) >> 7;
}

void libtime_init(void)
{
    const int NR_TIME_ITERS = 10;
    double delta, mean, S;
    uint32_t avg, cycles[NR_TIME_ITERS];
    int i, samples;

    cycles[0] = get_cycles_per_usec();
    S = delta = mean = 0.0;
    for (i = 0; i < NR_TIME_ITERS; i++) {
        cycles[i] = get_cycles_per_usec();
        delta = cycles[i] - mean;
        if (delta) {
            mean += delta / (i + 1.0);
            S += delta * (cycles[i] - mean);
        }
    }

    S = sqrt(S / (NR_TIME_ITERS - 1.0));

    samples = avg = 0;
    for (i = 0; i < NR_TIME_ITERS; i++) {
        double this = cycles[i];

        if ((fmax(this, mean) - fmin(this, mean)) > S)
            continue;
        samples++;
        avg += this;
    }

    S /= (double)NR_TIME_ITERS;
    mean /= 10.0;

    avg /= samples;
    avg = (avg + 9) / 10;

    cycles_per_usec = avg;
}

uint64_t libtime_wall(void)
{
#ifdef __MACH__
    static mach_timebase_info_data_t timebase;
    if (timebase.denom == 0)
        mach_timebase_info(&timebase);
    return (double)mach_absolute_time() * (double)timebase.numer / (double)timebase.denom;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000000ULL) + ts.tv_nsec;
#endif
}

uint64_t libtime_cpu_to_wall(uint64_t clock)
{
	if (!cycles_per_usec)
		libtime_init();
    return (clock * 1000ULL) / cycles_per_usec;
}
