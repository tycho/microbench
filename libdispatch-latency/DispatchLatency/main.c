//
//  main.c
//  DispatchLatency
//
//  Created by Steven Noonan on 12/11/13.
//  Copyright (c) 2013 Steven Noonan. All rights reserved.
//

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libtime.h"

#include <dispatch/dispatch.h>

//#define SORT
#ifdef SORT
static int dbl_compare(const void *a, const void *b)
{
    if (*(double *)a < *(double *)b) return -1;
    if (*(double *)a > *(double *)b) return 1;
    return 0;
}
#endif

void test_dispatch_latency(unsigned samples, unsigned sleeptime)
{
    unsigned i;

    uint64_t *lat = (uint64_t *)malloc(sizeof(uint64_t) * 3);
    uint64_t *history = (uint64_t *)malloc(sizeof(uint64_t) * samples);

    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_group_t group = dispatch_group_create();

    for (i = 0; i < samples; i++) {
        usleep(sleeptime);

        /*
         lat[0] to lat[1] is latency to block execution
         lat[1] to lat[2] is latency from block execution to block synchronization
         */
        lat[0] = libtime_cpu();
        dispatch_group_async(group, queue, ^{ lat[1] = libtime_cpu(); });
        dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
        lat[2] = libtime_cpu();

        /*
        printf("%04u lat[0] = %" PRIu64 "\n", i, lat[0]);
        printf("%04u lat[1] = %" PRIu64 ", %" PRIu64 "\n", i, lat[1], lat[1] - lat[0]);
        printf("%04u lat[2] = %" PRIu64 ", %" PRIu64 "\n", i, lat[2], lat[2] - lat[1]);
        */

        history[i] = lat[1] - lat[0];
        //history[i] = lat[2] - lat[1];
    }

    dispatch_release(group);

#ifdef SORT
    qsort(history, samples, sizeof(double), dbl_compare);
#endif

    printf("samples = %u, sleep time = %u usecs\n", samples, sleeptime);
    for (i = 0; i < samples; i++) {
        printf("history[%04u] = %" PRIu64 " nsec\n", i, libtime_cpu_to_wall(history[i]));
    }
    printf("\n");

    free(history);
    free(lat);
}

int main(int argc, char **argv)
{
    libtime_init();
    test_dispatch_latency(10, 0);
    test_dispatch_latency(10, 1);
    test_dispatch_latency(10, 10);
    test_dispatch_latency(10, 100);
    test_dispatch_latency(10, 1000);
    test_dispatch_latency(10, 10000);
    return 0;
}
