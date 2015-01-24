include common.mk

BINS := libdispatch-latency/dispatchlatency hz/hz popcnt/popcnt
DIRS := libdispatch-latency libtime hz popcnt

.PHONY: $(BINS) libtime/libtime.a

all: $(BINS)

$(BINS): libtime/libtime.a

clean:
	for DIR in $(DIRS); do \
		$(MAKE) -C $$DIR clean; \
	done

libtime/.git:
	git submodule update --init libtime

libtime/libtime.a: libtime/.git
	$(MAKE) -C libtime libtime.a

libdispatch-latency/dispatchlatency:
	$(MAKE) -C libdispatch-latency dispatchlatency

xen-hypercall/xen-hypercall:
	$(MAKE) -C xen-hypercall xen-hypercall

hz/hz:
	$(MAKE) -C hz hz

popcnt/popcnt:
	$(MAKE) -C popcnt popcnt
