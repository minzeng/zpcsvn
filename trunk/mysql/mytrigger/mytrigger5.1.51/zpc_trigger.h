#ifndef _ZPC_TRIGGER_H
#define _ZPC_TRIGGER_H

#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include "queue.h"
/* my log */
static FILE *log_file_p;
char *log_file = (char*)"./mytrigger.log";
#define MYLOG(fmt, arg...) \
	do { \
		fprintf(log_file_p, "%s[%d]: "#fmt"\n", __func__, __LINE__, ##arg);\
		fflush(log_file_p);\
	}while(0)

#define MYLOG_INIT() do {\
	if (access(log_file, F_OK)) {\
		log_file_p = fopen(log_file, "w");\
		if (NULL == log_file_p) {\
			error("creat log file[%s] faild", log_file);\
			exit(1);\
		}\
	} else {\
		log_file_p = fopen(log_file, "a");\
		if (NULL == log_file_p) {\
			error("open log file[%s] faild", log_file);	\
			exit(1);\
		}\
	}\
	}while(0)

ulong self_server_id = 777777777;

#define EVENT_QUEUE_SIZE 10000
/* event queue */
struct EVENT_ITEM {
	void *e;
	TAILQ_ENTRY(EVENT_ITEM) entries; 
};
TAILQ_HEAD(,EVENT_ITEM) event_q_head;
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qready = PTHREAD_COND_INITIALIZER;
uintmax_t qs = 0;
int enqueue(void *);
void *dequeue();

/* mystruct.h DJ */
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

char *conf_file = (char*)"./mytrigger.info";
static FILE *conf_file_fd;
char current_log_name[_POSIX_PATH_MAX + 1];
char master_host[_POSIX_PATH_MAX];
char master_user[_POSIX_PATH_MAX];
char master_pass[_POSIX_PATH_MAX];
bool is_daemonize = false;
bool skip_slave_error = 0;

char *mytrigger_so = (char*)"./mytrigger.so";

int daemonize(int nochdir, int noclose);
size_t strlcpy(char *dst, const char *src, size_t siz);

#endif
