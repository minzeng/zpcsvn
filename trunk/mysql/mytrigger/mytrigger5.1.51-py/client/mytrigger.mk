CC = gcc
IFLAGS =
CFLAGS = -ggdb -fPIC \
		-DSAFE_MUTEX -DSAFEMALLOC -Wall -Wextra -Wunused -Wwrite-strings \
		-Wno-unused-parameter -Wctor-dtor-privacy -Wnon-virtual-dtor \
		-felide-constructors -fno-exceptions -fno-rtti -m64 \
		-DUNIV_MUST_NOT_INLINE -DEXTRA_DEBUG -DFORCE_INIT_OF_VARS -DSAFEMALLOC \
		-DPEDANTIC_SAFEMALLOC -DSAFE_MUTEX -fno-implicit-templates \
		-fno-exceptions -fno-rtti -rdynamic
PREDEF =
LDFLAGS = -lpthread ../libmysql/.libs/libmysqlclient.a -lpthread -lcrypt \
		-lnsl -lm -lz -Wl,--rpath -Wl,/data0/zpc/mysql-bin-row/lib/mysql -ldl

LINK =
OBJS = mysqlbinlog.o mf_tempdir.o my_new.o my_bit.o my_bitmap.o \
		my_vle.o base64.o
WARN =

.PHONY: clean

.c.o:
	$(CC) $(WARN) -c $*.c $(IFLAGS) $(PREDEF)

mytrigger: mytrigger.o mytrigger-worker
	$(CC) $(IFLAGS) -o $@ $(DEBUG) $(OBJS) $(LINK) $(LDFLAGS)

mytrigger-worker: $(OBJS)
	$(CC) $(IFLAGS) -o $@ $(DEBUG) $(CFLAGS) $(OBJS) $(LINK) $(LDFLAGS)

clean:
	rm -f mytrigger-worker.o mytrigger-worker mytrigger.o mytrigger
