// gcc -O2 -o test_vmcall test_vmcall.c -lxenctrl -lrt -lm

#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "libtime.h"

static int compare_uint32(const void *pa, const void *pb)
{
	uint32_t a, b;
	a = *(uint32_t *)pa;
	b = *(uint32_t *)pb;
	if (a > b)
		return 1;
	if (a < b)
		return -1;
	return 0;
}

static const char *pretty_print_ns(uint64_t ns, uint32_t bar)
{
	static char buffer[64];
	double v = ns;
	const char *suffix = "ns";
	if (v > bar * 1000) {
		v /= 1000;
		suffix = "us";
	}
	if (v > bar * 1000) {
		v /= 1000;
		suffix = "ms";
	}
	if (v > bar * 1000) {
		v /= 1000;
		suffix = "s";
	}
	snprintf(buffer, sizeof(buffer), "%.2lf %s", v, suffix);
	return buffer;
}

static const char *pretty_print_hz(uint64_t hz, uint32_t bar)
{
	static char buffer[64];
	double v = hz;
	const char *suffix = "Hz";
	if (v > bar * 1000) {
		v /= 1000;
		suffix = "KHz";
	}
	if (v > bar * 1000) {
		v /= 1000;
		suffix = "MHz";
	}
	if (v > bar * 1000) {
		v /= 1000;
		suffix = "GHz";
	}
	snprintf(buffer, sizeof(buffer), "%.2lf %s", v, suffix);
	return buffer;
}

static void print_time(const char *caption, uint32_t cycles)
{
	uint32_t ns = libtime_cpu_to_wall(cycles);
	double   hz = 1e9 / (double)ns;

	printf("%s: %" PRIu32 " cycles, %s, %s\n",
		caption,
		cycles,
		pretty_print_ns(ns, 10),
		pretty_print_hz(hz, 10));
}

static void test_sleep(uint32_t samples, uint32_t hits, uint64_t duration)
{
	uint32_t *times = (uint32_t *)malloc(sizeof(uint32_t) * samples);
	uint32_t i, j;
	uint64_t s, e, avg;

	for (i = 0; i < samples; i++) {
		struct timespec ts;
		s = libtime_cpu();
		for (j = 0; j < hits; j++) {
			ts.tv_sec = 0;
			ts.tv_nsec = duration;
			clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
			//nanosleep(&ts, NULL);
			//usleep(duration / 1000);
		}
		e = libtime_cpu();
		times[i] = (uint32_t)(e - s) / hits;
	}

	qsort(times, samples, sizeof(uint32_t), compare_uint32);

	avg = 0;
	for (i = 0; i < samples; i++) {
		//char caption[16];
		//snprintf(caption, sizeof(caption), "times[%" PRIu32 "]", i);
		//print_time(caption, times[i]);
		avg += times[i];
	}

	avg /= samples;

	print_time("MIN", times[0]);
	print_time("MAX", times[samples - 1]);
	print_time("MED", times[samples / 2]);
	print_time("AVG", avg);

	free(times);
}

int main(int argc, char **argv)
{
	libtime_init();
	test_sleep(100, 1, 33000000);
	return 0;
}

