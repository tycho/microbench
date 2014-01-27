// gcc -O2 -o test_vmcall test_vmcall.c -lxenctrl -lrt -lm

#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <xenctrl.h>

#ifdef XENCTRL_HAS_XC_INTERFACE
#define XC_INTERFACE_BAD_HANDLE NULL
typedef xc_interface *xc_interface_t;
#else
#define xc_interface_open(x,y,z) xc_interface_open()
#define XC_INTERFACE_BAD_HANDLE -1
typedef int xc_interface_t;
#endif

static inline uint64_t get_cpu_clock(void)
{
    uint32_t lo, hi;

    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t) hi << 32ULL) | lo;
}

static uint32_t cycles_per_usec;

static uint64_t wallclock_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000000ULL) + ts.tv_nsec;
}

static uint32_t get_cycles_per_usec(void)
{
    uint64_t wc_s, wc_e;
    uint64_t c_s, c_e;

    wc_s = wallclock_ns();
    c_s = get_cpu_clock();
    do {
        uint64_t elapsed;

        wc_e = wallclock_ns();
        elapsed = wc_e - wc_s;
        if (elapsed >= 1280000ULL) {
            c_e = get_cpu_clock();
            break;
        }
    } while (1);

    return (c_e - c_s + 127) >> 7;
}

static void calibrate_cpu_clock(void)
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

uint64_t cpu_clock_to_wall(uint64_t clock)
{
    return (clock * 1000ULL) / cycles_per_usec;
}

int main(int argc, char **argv)
{
	int v, r = 0;
	uint32_t i, max = 1000000;
	uint64_t s, e;
	xc_interface_t xc_handle = xc_interface_open(NULL, NULL, 0);
	if (xc_handle == XC_INTERFACE_BAD_HANDLE) {
		fprintf(stderr, "xc_interface_open failed\n");
		r = -1;
		goto exit;
	}

	calibrate_cpu_clock();
	s = get_cpu_clock();
	for (i = 0; i < max; i++) {
		v = xc_version(xc_handle, XENVER_version, NULL);
		if (v < 0) {
			r = -1;
			goto cleanup;
		}
	}
	e = get_cpu_clock();
		
	printf("total: %" PRIu64 " ns (%" PRIu64 " clocks), ", cpu_clock_to_wall(e - s), e - s);
	printf("per call: %" PRIu64 " ns (%" PRIu64 " clocks)\n", cpu_clock_to_wall(e - s) / max, (e - s) / max);
	
cleanup:
	xc_interface_close(xc_handle);
exit:
	return r;
}

