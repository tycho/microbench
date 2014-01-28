// gcc -O2 -o test_vmcall test_vmcall.c -lxenctrl -lrt -lm

#include <math.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include <xenctrl.h>

#include "libtime.h"

#ifdef XENCTRL_HAS_XC_INTERFACE
#define XC_INTERFACE_BAD_HANDLE NULL
typedef xc_interface *xc_interface_t;
#else
#define xc_interface_open(x,y,z) xc_interface_open()
#define XC_INTERFACE_BAD_HANDLE -1
typedef int xc_interface_t;
#endif

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

	libtime_init();
	s = libtime_cpu();
	for (i = 0; i < max; i++) {
		v = xc_version(xc_handle, XENVER_version, NULL);
		if (v < 0) {
			r = -1;
			goto cleanup;
		}
	}
	e = libtime_cpu();
		
	printf("total: %" PRIu64 " ns (%" PRIu64 " clocks), ", libtime_cpu_to_wall(e - s), e - s);
	printf("per call: %" PRIu64 " ns (%" PRIu64 " clocks)\n", libtime_cpu_to_wall(e - s) / max, (e - s) / max);
	
cleanup:
	xc_interface_close(xc_handle);
exit:
	return r;
}

