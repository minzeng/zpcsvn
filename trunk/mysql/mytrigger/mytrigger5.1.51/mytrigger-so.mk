CC	= gcc
IFLAGS =
CFLAGS = -ggdb -fPIC
PREDEF =
LDFLAGS = -ggdb -shared libhiredis.a libconfparser.a

LINK = 
OBJS = mytriggerso.redis.o crc32.o
WARN =

.PHONY: clean

.c.o:	
	$(CC) $(WARN) -c $*.c $(CFLAGS) $(IFLAGS) $(PREDEF)

mytrigger.so: $(OBJS)
	$(CC) $(IFLAGS) -fPIC -o $@ $(DEBUG) $(OBJS) $(LINK) $(LDFLAGS)

clean:
	rm -f mytriggerso.redis.o crc32.o mytrigger.so
