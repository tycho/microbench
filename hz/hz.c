#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "libtime.h"

const char *time_suffixes[] = { "ns", "us", "ms", "s", NULL };
const char *rate_suffixes[] = { "Hz", "KHz", "MHz", "GHz", NULL };

static const char *pretty_print(char *buffer, size_t bufsz, double v,
		const char **suffixes, uint32_t bar)
{
	const char **suffix = suffixes;
	while (v > bar * 1000.0 && suffix[1]) {
		v /= 1000.0;
		suffix++;
	}
	snprintf(buffer, bufsz, "%.3lf %s", v, *suffix);
	return buffer;
}

uint64_t estimate_min_sleep(void)
{
	struct timespec ts;
	uint32_t i, j;
	uint64_t s, e, min;

	ts.tv_sec = 0;
	ts.tv_nsec = 0;

	min = (uint64_t)-1;

	for (j = 0; j < 100; j++) {
		s = libtime_cpu();
		for (i = 0; i < 100; i++) {
			clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
		}
		e = libtime_cpu();
		if ((e - s) < min)
			min = (e - s);
	}

	return libtime_cpu_to_wall(min) / 100;
}

int main(int argc, char **argv)
{
	struct timespec ts;
	uint64_t s, e, ns_elapsed, ns_target, tick;
	int64_t ns_to_sleep, min_sleep;
	char buf[2][32];

	/*
	 * The rate to enforce.
	 */
	const uint64_t hz = 30;

	/*
	 * Initialize our clocks.
	 */
	libtime_init();

	/*
	 * Find the minimum possible sleep duration.
	 */
	min_sleep = estimate_min_sleep();
	printf("Estimated minimum sleep time: %s\n",
		pretty_print(buf[0], sizeof(buf[0]), min_sleep, time_suffixes, 10));

	/*
	 * Our goal is for our iteration rate to match a certain frequency in Hz.
	 * Find the number of expected nanoseconds per iteration.
	 */
	ns_target = 1e9 / hz;

	/*
	 * Since we'll never sleep for >= 1 second, we can just initialize tv_sec
	 * to zero and leave it that way.
	 */
	ts.tv_sec = 0;

	tick = 0;

	while (1) {
		tick++;

		s = libtime_cpu();

		/******************************
		 * Do rate-limited work here. *
		 ******************************/

		e = libtime_cpu();

		/*
		 * Determine how long this iteration took, and whether or not we have
		 * any time to kill.
		 */
		ns_elapsed = libtime_cpu_to_wall(e - s);
		ns_to_sleep = ns_target - ns_elapsed;

		if (ns_elapsed > ns_target) {
			/* 
			 * This iteration went over the expected time per iteration. This
			 * means that our workload needs to change in order to fit the
			 * specified frequency.
			 */
			printf("Completed work in %s, past deadline of %s\n",
				pretty_print(buf[0], sizeof(buf[0]), ns_elapsed, time_suffixes, 10),
				pretty_print(buf[1], sizeof(buf[1]), ns_target, time_suffixes, 10));
		}

		while (ns_to_sleep > min_sleep) {
			/*
			 * We're under time, so sleep for the remainder of the
			 * timeslice.
			 */
			ts.tv_nsec = 0;
			clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);

			e = libtime_cpu();

			ns_elapsed = libtime_cpu_to_wall(e - s);
			ns_to_sleep = ns_target - ns_elapsed;
		}

		while (ns_to_sleep > 0) {
			/*
			 * We are very close to the time quantum, but we still have some
			 * time to burn. Spin until we run down the clock.
			 */
			e = libtime_cpu();

			ns_elapsed = libtime_cpu_to_wall(e - s);
			ns_to_sleep = ns_target - ns_elapsed;
		}

		/*
		 * Print statistics once every second.
		 */
		if (tick % hz == 0) {
			printf("Iteration completed in %s (%s)\n",
				pretty_print(buf[0], sizeof(buf[0]), ns_elapsed, time_suffixes, 10),
				pretty_print(buf[1], sizeof(buf[1]), 1e9 / (double)ns_elapsed, rate_suffixes, 1));
		}
	}

	return 0;
}

