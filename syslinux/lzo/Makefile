topdir = ..
MAKEDIR = $(topdir)/mk
include $(MAKEDIR)/build.mk

INCLUDES += -I./include
CFLAGS += -Wno-unused-local-typedefs
CFLAGS += -fno-strict-aliasing

all: prepcore

# Don't include $(UMAKEDEPS) here, it's not needed.
prepcore: prepcore.c src/lzo1x_o.c src/lzo_util.c src/lzo1x_9x.c src/lzo_init.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

tidy dist clean spotless:
	rm -f prepcore
