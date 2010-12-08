CC 	= gcc
IFLAGS =
CFLAGS =
PREDEF =
LDFLAGS = -shared

LINK =
OBJS = mytriggerso.c
WARN =

.PHONY: clean

.c.o:	
	$(CC) $(WARN) -c $*.c $(CFLAGS) $(IFLAGS) $(PREDEF)

mytrigger.so: $(OBJS)
	$(CC) $(IFLAGS) -fPIC -o $@ $(DEBUG) $(OBJS) $(LINK) $(LDFLAGS)

clean:
	rm -f mytrigger.o mytrigger.so

