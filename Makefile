BINS := libdispatch-latency/dispatchlatency xen-hypercall/xen-hypercall
DIRS := libdispatch-latency xen-hypercall time

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
