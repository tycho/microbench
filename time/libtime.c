#include "platform.h"

#if defined(TARGET_OS_MACOSX)
#include <mach/mach_time.h>
#elif defined(TARGET_OS_WINDOWS)
#include <windows.h>
#else
#include <time.h>
#endif
#include <stdint.h>
#include <math.h>

#include "libtime.h"

static uint32_t cycles_per_usec;
static uint64_t min_sleep_ns;
static uint64_t sleep_overhead_clk;
#if defined(TARGET_OS_MACOSX)
static mach_timebase_info_data_t timebase;
#elif defined(TARGET_OS_WINDOWS)
static LARGE_INTEGER perf_frequency;
#endif

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


static void libtime_init_wallclock(void)
{
#if defined(TARGET_OS_MACOSX)
    mach_timebase_info(&timebase);
#elif defined(TARGET_OS_WINDOWS)
    QueryPerformanceFrequency(&perf_frequency);
#endif
}

static void libtime_init_cpuclock(void)
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

static inline void _libtime_nanosleep(void)
{
#if defined(TARGET_OS_WINDOWS)
	/* Hrm. */
	Sleep(0);
#else
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 0;
#if defined(TARGET_OS_MACOSX)
	nanosleep(&ts, NULL);
#else
	clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
#endif
#endif
}

static void libtime_init_sleep(void)
{
	uint32_t i, j;
	uint64_t s, e, min;

#if defined(TARGET_OS_WINDOWS)
	timeBeginPeriod(1);
#endif

	/*
	 * Estimate the minimum time consumed by a nanosleep(0).
	 */
	min = (uint64_t)-1;
	for (j = 0; j < 100; j++) {
	    s = libtime_cpu();
	    for (i = 0; i < 128; i++) {
			_libtime_nanosleep();
	    }
	    e = libtime_cpu();
	    if ((e - s) < min)
			min = (e - s);
	}
	min_sleep_ns = libtime_cpu_to_wall((min + 127) >> 7);

	/*
	 * Estimate the minimum time consumed by libtime_nanosleep(0).
	 */
	min = (uint64_t)-1;
	for (j = 0; j < 100; j++) {
		s = libtime_cpu();
		for (i = 0; i < 128; i++) {
			libtime_nanosleep(0);
		}
		e = libtime_cpu();
	    if ((e - s) < min)
			min = (e - s);
	}
	sleep_overhead_clk = (min + 127) >> 7;
}

void libtime_init(void)
{
	libtime_init_wallclock();
	libtime_init_cpuclock();
	libtime_init_sleep();
}

uint64_t libtime_wall(void)
{
#if defined(TARGET_OS_MACOSX)
    return (double)mach_absolute_time() * (double)timebase.numer / (double)timebase.denom;
#elif defined(TARGET_OS_WINDOWS)
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (counter.QuadPart * 1000000000ULL) / perf_frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000000ULL) + ts.tv_nsec;
#endif
}

uint64_t libtime_cpu_to_wall(uint64_t clock)
{
    return (clock * 1000ULL) / cycles_per_usec;
}

void libtime_nanosleep(int64_t ns)
{
	uint64_t s, e;
	uint64_t ns_elapsed;
	int64_t ns_to_sleep;

	/*
	 * Our goal is to sleep as close to 'ns' nanoseconds as possible. To
	 * accomplish this, we use the system nanosleep functionality until another
	 * sleep would take us over our quantum. Then we spin until we run the
	 * clock down.
	 */
	s = libtime_cpu() - sleep_overhead_clk;
	do {
		e = libtime_cpu();

		ns_elapsed = libtime_cpu_to_wall(e - s);
		ns_to_sleep = ns - ns_elapsed;

		if (ns_to_sleep > min_sleep_ns) {
			_libtime_nanosleep();
		}

	} while (ns_elapsed < ns);
}
