include common.mk

BINS := libdispatch-latency/dispatchlatency hz/hz popcnt/popcnt
DIRS := libdispatch-latency time hz popcnt

.PHONY: $(BINS) time/libtime.a

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

popcnt/popcnt:
	$(MAKE) -C popcnt popcnt
