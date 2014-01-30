#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "libtime.h"

const char *time_suffixes[] = { "ns", "us", "ms", "s", NULL };
const char *rate_suffixes[] = { "Hz", "KHz", "MHz", "GHz", NULL };

static const char *pretty_print(char *buffer, size_t bufsz, uint64_t uv,
		const char **suffixes, uint32_t bar)
{
	double v = uv;
	const char **suffix = suffixes;
	while (v > bar * 1000 && suffix[1]) {
		v /= 1000;
		suffix++;
	}
	snprintf(buffer, bufsz, "%.2lf %s", v, *suffix);
	return buffer;
}

int main(int argc, char **argv)
{
	struct timespec ts;
	uint64_t s, e, ns_elapsed, ns_target;
	const uint64_t hz = 30;
	int64_t ns_to_sleep;
	char buf[2][32];

	/*
	 * Initialize our clocks.
	 */
	libtime_init();

	/*
	 * Our goal is for our iteration rate to match a certain frequency in Hz.
	 * Find the number of expected nanoseconds per iteration.
	 */
	ns_target = 1e9 / (hz + 1);

	/*
	 * Since we'll never sleep for >= 1 second, we can just initialize tv_sec
	 * to zero and leave it that way.
	 */
	ts.tv_sec = 0;

	while (1) {

		s = libtime_cpu();

		/*
		 * Do rate-limited work here.
		 */

		e = libtime_cpu();

		/*
		 * Determine how long this iteration took, and whether or not we have
		 * any time to kill.
		 */
		ns_elapsed = libtime_cpu_to_wall(e - s);
		ns_to_sleep = ns_target - ns_elapsed;

		if (ns_to_sleep > 0) {
			/*
			 * We're under time, so sleep for the remainder of the
			 * timeslice.
			 */
			ts.tv_nsec = ns_to_sleep;
			clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
		} else {
			/* 
			 * This iteration went *over* the expected time per iteration. This
			 * means that our workload needs to change in order to fit the
			 * specified frequency.
			 */
			printf("Completed work in %s, more than %s, not sleeping\n",
				pretty_print(buf[0], sizeof(buf[0]), ns_elapsed, time_suffixes, 10),
				pretty_print(buf[1], sizeof(buf[1]), ns_target, time_suffixes, 10));
		}

		/* 
		 * Recalculate how much time we've consumed. If our sleep function is
		 * sleeping too long or too little, this will clue us in.
		 */
		e = libtime_cpu();
		ns_elapsed = libtime_cpu_to_wall(e - s);

		printf("Iteration completed in %s (%s)\n",
			pretty_print(buf[0], sizeof(buf[0]), ns_elapsed, time_suffixes, 10),
			pretty_print(buf[1], sizeof(buf[1]), 1e9 / (double)ns_elapsed, rate_suffixes, 1));
	}

	return 0;
}

