#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
struct MY_DATA {
	void *data;
	int type;
	int length;
};

struct TRIGGER_DATA {
	int ioperate_type;
	char *logfile;
	unsigned long log_pos;
	int filednum;
	char* dbname;
	char* tbname;
	struct MY_DATA* row_list;
	struct MY_DATA* row_list_update;
};

