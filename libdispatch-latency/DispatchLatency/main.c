//
//  main.c
//  DispatchLatency
//
//  Created by Steven Noonan on 12/11/13.
//  Copyright (c) 2013 Steven Noonan. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <mach/mach_time.h>
#include <dispatch/dispatch.h>

static double clock_read_time(void)
{
    static mach_timebase_info_data_t timebase;
    if (timebase.denom == 0)
        mach_timebase_info(&timebase);
    return (double)mach_absolute_time() * (double)timebase.numer / (double)timebase.denom;
}

static int dbl_compare(const void *a, const void *b)
{
    if (*(double *)a < *(double *)b) return -1;
    if (*(double *)a > *(double *)b) return 1;
    return 0;
}

void test_dispatch_latency(unsigned samples, unsigned sleeptime)
{
    unsigned i;

    double *lat = (double *)malloc(sizeof(double) * 3);
    double *history = (double *)malloc(sizeof(double) * samples);

    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_group_t group = dispatch_group_create();

    for (i = 0; i < samples; i++) {
        usleep(sleeptime);

        /*
         lat[0] to lat[1] is latency to block execution
         lat[1] to lat[2] is latency from block execution to block synchronization
         */
        lat[0] = clock_read_time();
        dispatch_group_async(group, queue, ^{ lat[1] = clock_read_time(); });
        dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
        lat[2] = clock_read_time();

        /*
        printf("%04u lat[0] = %.0lf\n", i, lat[0]);
        printf("%04u lat[1] = %.0lf, %.0lf\n", i, lat[1], lat[1] - lat[0]);
        printf("%04u lat[2] = %.0lf, %.0lf\n", i, lat[2], lat[2] - lat[1]);
        */

        history[i] = lat[1] - lat[0];
        //history[i] = lat[2] - lat[1];
    }

    dispatch_release(group);

    qsort(history, samples, sizeof(double), dbl_compare);

    printf("samples = %u, sleep time = %u usecs\n", samples, sleeptime);
    for (i = 0; i < samples; i++) {
        printf("history[%04u] = %.0lf\n", i, history[i]);
    }
    printf("\n");

    free(history);
    free(lat);
}

int main(int argc, char **argv)
{
    test_dispatch_latency(10, 0);
    test_dispatch_latency(10, 1);
    test_dispatch_latency(10, 10);
    test_dispatch_latency(10, 100);
    test_dispatch_latency(10, 1000);
    test_dispatch_latency(10, 10000);
    return 0;
}
