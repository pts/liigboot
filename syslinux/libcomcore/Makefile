MAKEFLAGS += -r
MAKE      += -r

topdir = ..
MAKEDIR = $(topdir)/mk
include $(MAKEDIR)/lib.mk

LIBCOREOBJS =   \
    memcpy.o mempcpy.o memset.o memcmp.o memmove.o                  \
    strlen.o stpcpy.o strcpy.o strcmp.o strlcpy.o strlcat.o         \
    strchr.o strncmp.o strncpy.o                                    \
    snprintf.o sprintf.o vsnprintf.o                                \
    dprintf.o vdprintf.o                                            \
    zalloc.o strdup.o                                               \
    sys/intcall.o sys/farcall.o sys/cfarcall.o sys/zeroregs.o       \
    libgcc/__ashldi3.o libgcc/__udivdi3.o                           \
    libgcc/__negdi2.o libgcc/__ashrdi3.o libgcc/__lshrdi3.o         \
    libgcc/__muldi3.o libgcc/__udivmoddi4.o libgcc/__umoddi3.o      \
    libgcc/__divdi3.o libgcc/__moddi3.o

HEADERS := $(wildcard include/*.h include/bitsize/*.h include/klibc/*.h include/sys/*.h)

# Don't include $(MAKEDEPS) or $(UMAKEDEPS) here, it's not needed.
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

libcomcore.a: $(LIBCOREOBJS)
	rm -f $@
	$(AR) cq $@ $^
	$(RANLIB) $@  

clean:
	rm -f libcomcore.a $(LIBCOREOBJS) .*.o.d sys/.*.o.d libgcc/.*.o.d


