build:
	1£¬½øÈëclientÄ¿Â¼
	2£¬make
	3£¬make -f mytrigger.mk
	4£¬make -f so_call_py.mk

startup:
	./mytrigger --defaults-file=mytrigger.cnf mysql-bin.000074
	