CC 	= gcc
IFLAGS =
CFLAGS = -ggdb
PREDEF =
LDFLAGS = -shared

LINK =
OBJS = mytriggerso.redis.c libhiredis.a
WARN =

.PHONY: clean

.c.o:	
	$(CC) $(WARN) -c $*.c $(CFLAGS) $(IFLAGS) $(PREDEF)

mytrigger.so: $(OBJS)
	$(CC) $(IFLAGS) -fPIC -o $@ $(DEBUG) $(OBJS) $(LINK) $(LDFLAGS)

clean:
	rm -f mytriggerso.redis.o mytrigger.so

