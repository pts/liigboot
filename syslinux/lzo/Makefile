topdir = ..
MAKEDIR = $(topdir)/mk
include $(MAKEDIR)/build.mk

CFLAGS += -Wno-unused-local-typedefs
CFLAGS += -fno-strict-aliasing

all: prepcore

# Don't include $(UMAKEDEPS) here, it's not needed.
prepcore: prepcore.c prepcore_lzo.c prepcore_lzo.h prepcore_lzo_miniacc.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ prepcore.c prepcore_lzo.c $(LIBS)

tidy dist clean spotless:
	rm -f prepcore
