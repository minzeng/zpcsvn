CC	= gcc
IFLAGS = -I/usr/local/include/python2.6/
CFLAGS = -ggdb -fPIC -shared
PREDEF =
LDFLAGS =  -shared -ggdb -lm -ldl -lpthread -lutil -lpython2.6 \
		-Xlinker -export-dynamic /usr/local/lib/libjson.a

LINK =
OBJS = so_call_py.o
WARN =

.PHONY: clean

.c.o:
	$(CC) $(WARN) -c $*.c $(CFLAGS) $(IFLAGS) $(PREDEF)

so_call_py.so: $(OBJS)
	$(CC) $(IFLAGS) -fPIC -o $@ $(DEBUG) $(OBJS) $(LINK) $(LDFLAGS)

clean:
	rm -f so_call_py.o so_call_py.so
