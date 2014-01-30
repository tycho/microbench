MAKEFLAGS := -Rr

BINS := libdispatch-latency/dispatchlatency xen-hypercall/xen-hypercall hz/hz
DIRS := libdispatch-latency xen-hypercall time hz

all: $(BINS)

$(BINS): time/libtime.a

clean:
	for DIR in $(DIRS); do \
		$(MAKE) -C $$DIR clean; \
	done

time/libtime.a:
	$(MAKE) -C time libtime.a

libdispatch-latency/dispatchlatency:
	$(MAKE) -C libdispatch-latency dispatchlatency

xen-hypercall/xen-hypercall:
	$(MAKE) -C xen-hypercall xen-hypercall

hz/hz:
	$(MAKE) -C hz hz
