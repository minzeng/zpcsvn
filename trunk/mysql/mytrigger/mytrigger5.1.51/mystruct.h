#ifndef __MYSTRUCT_H__
#define __MYSTRUCT_H__

#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#define ERR_MSG_LEN 1024

#define LOG(err_msg, fmt, arg...) \
	do { \
		if (NULL == err_msg) break; \
		snprintf(err_msg, ERR_MSG_LEN, \
			"%s[%d]: "#fmt"\n", __func__, __LINE__, ##arg);\
	}while(0)

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
	int b_islast;
};

#endif
