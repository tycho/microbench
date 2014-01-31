#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

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

int main(int argc, char **argv)
{
	uint64_t s, e, ns_elapsed, ns_target, tick;
	int64_t ns_to_sleep;
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
	 * Our goal is for our iteration rate to match a certain frequency in Hz.
	 * Find the number of expected nanoseconds per iteration.
	 */
	ns_target = 1e9 / hz;

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

		if (ns_to_sleep < 0) {

			/* 
			 * This iteration went over the expected time per iteration. This
			 * means that our workload needs to change in order to fit the
			 * specified frequency.
			 */
			printf("Completed work in %s, past deadline of %s\n",
				pretty_print(buf[0], sizeof(buf[0]), ns_elapsed, time_suffixes, 10),
				pretty_print(buf[1], sizeof(buf[1]), ns_target, time_suffixes, 10));

		} else if (ns_to_sleep > 0) {

			/*
			 * Do a high-precision sleep until our quantum expries.
			 */
			libtime_nanosleep(ns_to_sleep);
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

