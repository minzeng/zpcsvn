/*-
 * Copyright (c) 2010-2011 SINA Corporation, All Rights Reserved.
 * Author: Zhao pengcheng <pengcheng2@staff.sina.com.cn>
 */
#ifndef _ZPC_TRIGGER_H
#define _ZPC_TRIGGER_H

#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include "queue.h"
#include "row_data.h"
#include "zpc_log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mytrigger_server {
	int verbosity;
	char *log_level;
	char *log_file;
	char *python_script;
	char *master_info_file;
	char *mytrigger_so_file;
	unsigned long self_server_id;
	unsigned long row_queue_size;
	int is_daemonize;
	int skip_slave_error;
	char master_host[_POSIX_PATH_MAX];
	char master_user[_POSIX_PATH_MAX];
	char master_pass[_POSIX_PATH_MAX];
} MYTRIGGER_SERVER;

static MYTRIGGER_SERVER server;
FILE* master_info_file_p;
static int stmt_index_start;
static int stmt_index;

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

char current_log_name[_POSIX_PATH_MAX + 1];
static short shutdown_flag = 0;
static short main_shutdown_flag = 0;

int daemonize(int nochdir, int noclose);
size_t strlcpy(char *dst, const char *src, size_t siz);

#ifdef __cplusplus
} // extern "C"
#endif
#endif /* #ifndef _ZPC_TRIGGER_H */
