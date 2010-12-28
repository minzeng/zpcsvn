#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> /* for va_list */
#include <limits.h>
#include <time.h>

#include "mystruct.h"
/* redis client lib */
#include "hiredis.h"
#include "tree.h"
#include "confparser.h"

/*extern from mytrigger daemon */
extern char *redis_conf_file;
extern char *conf_file;
extern const char* host;
extern const char* user;
extern char* pass;
extern ulong self_server_id;
extern int port;
extern char **myargv;

/* my log from main program */
extern FILE *log_file_p;
#define MYLOG(fmt, arg...) \
	do {  \
		char *asc_time; time_t timeticks; \
		timeticks = time(NULL);\
		asc_time = asctime(localtime(&timeticks)); \
		asc_time[strlen(asc_time) - 1] = '\0';  /* delete the last '\n' */ \
		fprintf(log_file_p, "%s[%d]%s: "#fmt"\n", __func__, __LINE__, asc_time, ##arg);\
		fflush(log_file_p);\
	}while(0)

#define PRINT(fmt, arg...) \
	printf("%s[%d]: "#fmt"\n", __func__, __LINE__, ##arg)

#define RBUf_LEN 1024

/* 
 * RB-TREE store redis conf
 */
typedef struct redis_conf {
	char key[3*CONF_ITEM_LEN + 1]; /* dbname.tablename */

	/* [db.tb.common] */
	int redis_key;
	int redis_field;
	int redis_value;
	char redis_value_type[CONF_ITEM_LEN + 1];
	int mod;

	/* [db.tb.N] */
	char redis_host[CONF_ITEM_LEN + 1];
	int redis_port;
	int redis_db;
	int retry_times;
	int retry_interval;

	/* connect redis */
	redisContext *c;

	RB_ENTRY(redis_conf) rbt_entries;
} REDIS_CONF;

/*
 * compare function for RB-tree
 */
int
cmp(REDIS_CONF *l, REDIS_CONF *r) {
	return strncmp(l->key, r->key, 3*CONF_ITEM_LEN);
}

RB_HEAD(redis_conf_rb, redis_conf) redis_conf_rb_root;
RB_PROTOTYPE_STATIC(redis_conf_rb, redis_conf, rbt_entries, cmp);
RB_GENERATE_STATIC(redis_conf_rb, redis_conf, rbt_entries, cmp);

size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

int
load_redis_map_conf(const char *file) {
	dictionary *conf;
	int nsec;
	char *secname;
	int seclen;
	int i;
	int ret;
	REDIS_CONF *rc;

	conf = open_conf_file(file);
	if (NULL == conf) {
		MYLOG("cannot parse configure file: %s\n", file);
		return -1;
	}
	RB_INIT(&redis_conf_rb_root);
	nsec = iniparser_getnsec(conf);
	for (i = 0; i < nsec; i++) {
		secname = iniparser_getsecname(conf, i);
		seclen = (int)strlen(secname);

		rc = (REDIS_CONF *)calloc(1, sizeof(REDIS_CONF));
		if (NULL == rc) {
			MYLOG("alloc REDIS_CONF faild!");
			return -2;
		}
		/* store key */
		strlcpy(rc->key, secname, 3*CONF_ITEM_LEN + 1);
		CONF_INT_CONFIG redis_conf_int_array[] = {
			{"redis_port", &rc->redis_port},
			{"redis_db", &rc->redis_db},
			{"redis_key", &rc->redis_key},
			{"redis_field", &rc->redis_field},
			{"redis_value", &rc->redis_value},
			{"mod", &rc->mod},
			{"retry_times", &rc->retry_times},
			{"retry_interval", &rc->retry_interval},
			{0, 0}
		};
		CONF_STR_CONFIG redis_conf_str_array[] = {
			{"redis_host", rc->redis_host},
			{"redis_value_type", rc->redis_value_type},
			{0, 0}
		};

		ret = load_conf((char*)file, secname, redis_conf_int_array,
			redis_conf_str_array);
		if (ret != 0) {
			MYLOG("load configure[file: %s, section: %s] failed!",
					file, secname);
			return -1;
		}
		#ifdef DEBUG
		PRINT("section name: %s", secname);
		PRINT("    tredis_host: %s", rc->redis_host);
		PRINT("    redis_port: %d", rc->redis_port);
		PRINT("    redis_db: %d", rc->redis_db);
		PRINT("    redis_value_type: %s", rc->redis_value_type);
		PRINT("    redis_key: %d", rc->redis_key);
		PRINT("    redis_field: %d", rc->redis_field);
		#endif
		/* connect to redis */
		/*rc->c = redisConnect(redis_host, redis_port);
		if (rc->c->err) {
			MYLOG("Connection error[%s:%d]: %s\n", 
			redis_host, redis_port, c->errstr);
			return -3;
		}*/
		/* inser to RB-TREE */
		REDIS_CONF *temp = RB_INSERT(redis_conf_rb, &redis_conf_rb_root, rc);
		if (temp == NULL) {
			// OK
		} else {
			// temp duplicated
			MYLOG("duplicate key[%s]", secname);
		}
	}
	close_conf_file(conf);
	return 0;
}

/*
 * get redis common configure from RB-tree
 */
REDIS_CONF *
get_redis_common_conf(char *dbname, char *tbname){
	REDIS_CONF *find;
	REDIS_CONF temp;

	snprintf(temp.key, 3*CONF_ITEM_LEN + 1, "%s.%s.common", dbname, tbname);
	find = RB_FIND(redis_conf_rb, &redis_conf_rb_root, &(temp));
	return find;
}

/*
 * my redisCommand implementation 
 * contain reconnect mechanism
 */
void *myRedisCommand(REDIS_CONF *rcn, const char *format, ...) {
	va_list ap;
	void *reply = NULL;
	int retry_times = 0;

	va_start(ap,format);
	for(;;) {
		reply = redisvCommand(rcn->c, format, ap);
		if (NULL == reply) { /* need reconect */
			if (rcn->c->err) {
				MYLOG("execute redisvCommand faild![%s]", rcn->c->errstr);
			} else {
				MYLOG("execute redisvCommand faild![unknown error]");
			}
			for (;;) {
				/* Once an error is returned the context cannot be reused 
				   and you should set up a new connection. */
				redisFree(rcn->c);
				rcn->c = NULL;
				/* reconnect to redis */
				MYLOG("Reconnect to host:%s, port:%d]", 
					rcn->redis_host, rcn->redis_port);
				rcn->c = redisConnect(rcn->redis_host, rcn->redis_port);
				if (rcn->c->err) {
					MYLOG("Connection error[%s:%d]: %s\n", 
					rcn->redis_host, rcn->redis_port, rcn->c->errstr);
					/* sleep */
					usleep(rcn->retry_interval);
					retry_times++;
					if (retry_times >= rcn->retry_times) {
						MYLOG("retry %d times, now mytrigger stop!", rcn->retry_times);	
						exit(3);
						//return NULL;
					}
					continue;
				} else {
					/*  restart mytrigger */
					restart_mytrigger(rcn);
					break;
				}
			}
		} else {
			break;
		}
	}
	va_end(ap);
	return reply;
}

/*
 * confirm logname and position,
 * restart mytrigger process
 */
int
restart_mytrigger(REDIS_CONF *rcn) {
	redisReply *reply;
	char log_name[_POSIX_PATH_MAX + 1];
	char position[_POSIX_PATH_MAX + 1];
	pid_t pid;

	MYLOG("restart mytrigger...");
	/* get logname and position from redis */
	reply = myRedisCommand(rcn, "SELECT 0");
	if (reply->type == REDIS_REPLY_ERROR) {
		MYLOG("myRedisCommand error[%s]", reply->str);
		MYLOG("mytrigger stop");
		exit(1);
	}
	freeReplyObject(reply);
	reply = myRedisCommand(rcn, "GET logname");
	if (reply->type != REDIS_REPLY_STRING) {
		MYLOG("remove %s file", conf_file);
		if (remove(conf_file)){
			MYLOG("remove  %s file faild", conf_file);	
			MYLOG("mytrigger stop");
			exit(1);
		}
		goto restart;
	}
	strlcpy(log_name, reply->str, _POSIX_PATH_MAX + 1);
	freeReplyObject(reply);
	reply = myRedisCommand(rcn, "GET position");
	if (reply->type != REDIS_REPLY_STRING) {
		MYLOG("remove %s file", conf_file);
		if (remove(conf_file)){
			MYLOG("remove  %s file faild", conf_file);	
			MYLOG("mytrigger stop");
			exit(1);
		}
		goto restart;
	}
	strlcpy(position, reply->str, _POSIX_PATH_MAX + 1);
	freeReplyObject(reply);
	/* modify mytrigger.info file */
	char tmp[_POSIX_PATH_MAX];
	snprintf(tmp, _POSIX_PATH_MAX, "%s.tmp", conf_file);
	FILE *ft = fopen(tmp, "w");
	if (NULL == ft) {
		MYLOG("create tmp mytrigger info file[%s] faild, "
			"mytrigger exit!", tmp);
		exit(1);
	}																		           
	fprintf(ft, "%s\n", log_name);
	fprintf(ft, "%s\n", position);
	fprintf(ft, "%s\n", host);
	fprintf(ft, "%s\n", user);
	fprintf(ft, "%s\n", pass);
	fprintf(ft, "%d\n", port);
	fprintf(ft, "%ju\n", (uintmax_t)self_server_id);
	fclose(ft);
	if (rename(tmp, conf_file) != 0) {
		MYLOG("rename(%s, %s) faild, mytrigger exit!", tmp, conf_file);
		exit(1);
	}
	/* restart mytrigger */
restart:
	pid = fork();
	if(pid == 0) {
		execvp(myargv[0], myargv);
		MYLOG("ERROR: Could not execute %s", myargv[0]);
	} else {
		MYLOG("old mytrigger stop");
		exit(0);
	}
	return 0;
}

int
select_db(REDIS_CONF *rcn) {
	redisReply *reply;

	/* implement reconnect after */
	char valbuf[RBUf_LEN];
	snprintf(valbuf, RBUf_LEN, "%d", rcn->redis_db);
	reply = myRedisCommand(rcn, "SELECT %s", valbuf);
	if (reply->type == REDIS_REPLY_ERROR) {
		MYLOG("myRedisCommand error[%s]", reply->str);
		freeReplyObject(reply);
		return -1;
	}
	freeReplyObject(reply);
	return 0;
}

/*
 * get redis server configure from RB-tree
 */
REDIS_CONF *
get_redis_server_conf(char *dbname, char *tbname, uint32_t mod){
	REDIS_CONF *find;
	REDIS_CONF temp;
	int retry_times = 0;
	/* unit is microsecond */

	snprintf(temp.key, 3*CONF_ITEM_LEN + 1, "%s.%s.%u", dbname, tbname, mod);
	find = RB_FIND(redis_conf_rb, &redis_conf_rb_root, &(temp));
	if (NULL != find && NULL == find->c) {
		/* connect to redis */
		for (;;) {
			/* reconnect to redis */
			MYLOG("Reconnect to host:%s, port:%d]",
				find->redis_host, find->redis_port);
			find->c = redisConnect(find->redis_host, find->redis_port);
			if (find->c->err) {
				MYLOG("Connection error[%s:%d]: %s\n", 
				find->redis_host, find->redis_port, find->c->errstr);
				/* Once an error is returned the context cannot be reused 
				and you should set up a new connection. */
				redisFree(find->c);
				find->c = NULL;
				/* sleep */
				MYLOG("retry_interval: %d", find->retry_interval);
				usleep(find->retry_interval);
				retry_times++;
				if (retry_times >= find->retry_times) {
					MYLOG("retry %d times, now mytrigger stop!", find->retry_times);	
					exit(3);
					//return NULL;
				}
				continue;
			} else {
				break;
			}
		}
	}
	return find;
}

enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
			MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
			MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
			MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
			MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
			MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
			MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
			MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
			MYSQL_TYPE_BIT,
                        MYSQL_TYPE_NEWDECIMAL=246,
			MYSQL_TYPE_ENUM=247,
			MYSQL_TYPE_SET=248,
			MYSQL_TYPE_TINY_BLOB=249,
			MYSQL_TYPE_MEDIUM_BLOB=250,
			MYSQL_TYPE_LONG_BLOB=251,
			MYSQL_TYPE_BLOB=252,
			MYSQL_TYPE_VAR_STRING=253,
			MYSQL_TYPE_STRING=254,
			MYSQL_TYPE_GEOMETRY=255
};

int
init_proc(char *err_msg) {
	int re;
	re = load_redis_map_conf(redis_conf_file);
	if (re != 0) {
		MYLOG("load redis map configure faild[re=%d]", re);
		return -1;
	}
	return 0;
}

int
deinit_proc(char *err_msg) {
	/* implement affter*/
	//redisFree(c);
	return 0;
}

int
db_value2string(struct MY_DATA *md, char *buf) {
	int re;

	if (NULL == md->data) {
		MYLOG("NO DATA[type: %d]", md->type);
		return -1;
	}
	switch(md->type) {
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_LONG:
			re = snprintf(buf, RBUf_LEN, "%d", *((int *)md->data));
			break;
		case MYSQL_TYPE_FLOAT:
			re = snprintf(buf, RBUf_LEN, "%f", *((float *)md->data));
			break;
		case MYSQL_TYPE_DOUBLE:
			re = snprintf(buf, RBUf_LEN, "%f", *((double *)md->data));
			break;
		case MYSQL_TYPE_VARCHAR:
			re = snprintf(buf, RBUf_LEN, "%s", md->data);
			break;
		case MYSQL_TYPE_BLOB:
			re = snprintf(buf, RBUf_LEN, "%s", md->data);
			break;
		case MYSQL_TYPE_DATETIME:
			re = snprintf(buf, RBUf_LEN, "%s", md->data);
			break;
		case MYSQL_TYPE_LONGLONG:
			re = snprintf(buf, RBUf_LEN, "%lld", *((long long *)md->data));
			break;
		case MYSQL_TYPE_TIMESTAMP:
			if(md->length > 0) {
				re = snprintf(buf, RBUf_LEN, "%lld", *((long long *)md->data));
			} else {
				re = snprintf(buf, RBUf_LEN, "%s", md->data);
			}
			break;
		case MYSQL_TYPE_STRING:
			re = snprintf(buf, RBUf_LEN, "%s", md->data);
			break;
		case MYSQL_TYPE_TINY:
			re = snprintf(buf, RBUf_LEN, "%d", *((int *)md->data));
			break;
		case MYSQL_TYPE_DATE:
			if(md->length > 0) {
				re = snprintf(buf, RBUf_LEN, "%lld", *((long long *)md->data));
			} else {
				re = snprintf(buf, RBUf_LEN, "%s", md->data);
			}
			break;
		default:
			re = snprintf(buf, RBUf_LEN, "unknow type[%d]", md->type);
			return -1;
	}
	if (re >= RBUf_LEN) {
		return -re;
	} else {
		return re;
	}
}

int record_position(REDIS_CONF *rcn, struct TRIGGER_DATA* data) {
	redisReply *reply;
	/* implement reconnect after */
	char keybuf[RBUf_LEN];
	char valbuf[RBUf_LEN];

	if (!data->b_islast) {
		return 0;	
	}
	/* select db0 for record logname and position */
	reply = myRedisCommand(rcn, "SELECT 0");
	if (reply->type == REDIS_REPLY_ERROR) {
		MYLOG("myRedisCommand error[%s]", reply->str);
	}
	freeReplyObject(reply);

	snprintf(keybuf, RBUf_LEN, "logname");
	snprintf(valbuf, RBUf_LEN, "%s", data->logfile);
	reply = myRedisCommand(rcn, "SET %s %s", keybuf, valbuf);
	if (reply->type == REDIS_REPLY_ERROR) {
		MYLOG("myRedisCommand error[%s]", reply->str);
	}
	freeReplyObject(reply);

	snprintf(keybuf, RBUf_LEN, "position");
	snprintf(valbuf, RBUf_LEN, "%lu", data->log_pos);
	reply = myRedisCommand(rcn, "SET %s %s", keybuf, valbuf);
	if (reply->type == REDIS_REPLY_ERROR) {
		MYLOG("myRedisCommand error[%s]", reply->str);
	}
	freeReplyObject(reply);

	return 0;
}

int
i_proc(struct TRIGGER_DATA* data) {
	redisReply *reply;
	char rkey[RBUf_LEN];
	char num[RBUf_LEN];
	char buf[RBUf_LEN];
	int re;
	REDIS_CONF *rc;
	REDIS_CONF *rcn;
	int i=0;
	uint32_t N; /* crc-key % mod */
	uint32_t crc;

	/* get redis common configure from RB-TREE */
	rc = get_redis_common_conf(data->dbname, data->tbname);
	if (NULL == rc) {
		//MYLOG("redis common configure[%s.%s.common] not found!",
			//data->dbname, data->tbname);
		return -1;
	}
	/* check redis_key */
	if (rc->redis_key < 0 || rc->redis_key >= data->filednum) {
		MYLOG("invalid redis_key[section: %s.%s.common, redis_key: %d]",
			data->dbname, data->tbname, rc->redis_key);
		return -1;
	}
	/* determine redis key */
	if(NULL == data->row_list[rc->redis_key].data) {
		MYLOG("key is NULL[section: %s.%s.common, redis_key: %d]",
			data->dbname, data->tbname, rc->redis_key);
		return -1;
	}
	/* get value string */
	re = db_value2string(&data->row_list[rc->redis_key], rkey);
	if (re < 0) {
		MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, rkey);
		return -1;
	}
	/* hash function is crc32 */
	crc = mycrc32((uint8_t*)rkey, re);
	if (rc->mod <=0 ) {
		MYLOG("invalid mod[%d]", rc->mod);
		return -1;
	}
	N = crc % rc->mod;
	/* get redis server configure, contain connection */
	rcn = get_redis_server_conf(data->dbname, data->tbname, N);
	if (NULL == rcn) {
		//MYLOG("redis server configure[%s.%s.%u] not found!",
		//	data->dbname, data->tbname, N);
		return -1;
	}
	/* select redis db */
	if (select_db(rcn)) {
		MYLOG("select redis db faild!");
		return -1;
	}

	/* process data */
	if (!strncmp("row", rc->redis_value_type, sizeof("row") - 1)) { /* row data */
		for(i=0 ; i < data->filednum; i++) {
			if (rc->redis_key == i) { /* skip key */
				continue;
			}
			snprintf(num, RBUf_LEN, "%d", i); /* hset field */
			re = db_value2string(&data->row_list[i], buf);
			if (re < 0) {
				MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, buf);
				return -1; /*?*/
			}
			/* implement reconnect after */
			reply = myRedisCommand(rcn, "HSET %s %s %b", rkey, num, buf, re);
			if (reply->type == REDIS_REPLY_ERROR) {
				MYLOG("myRedisCommand error[%s]", reply->str);
			}
			freeReplyObject(reply);
		}
	} else if (!strncmp("list", rc->redis_value_type, sizeof("list") - 1)){ /* row data */
		/* check redis_value */
		if (rc->redis_value < 0 || rc->redis_value >= data->filednum) {
			MYLOG("invalid redis_value[section: %s.%s.common, redis_key: %d]",
				data->dbname, data->tbname, rc->redis_value);
			return -1;
		}
		re = db_value2string(&data->row_list[rc->redis_value], buf);
		if (re < 0) {
			MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, buf);
			return -1; /*?*/
		}
		/* implement reconnect after */
		reply = myRedisCommand(rcn, "SADD %s %s", rkey, buf);
		if (reply->type == REDIS_REPLY_ERROR) {
			MYLOG("myRedisCommand error[%s]", reply->str);
		}
		freeReplyObject(reply);
	} else { /* exception */
		MYLOG("nonsupport redis value type[%s]", rc->redis_value_type);
		return -1;
	}

	record_position(rcn, data);

	return  0;
}

int 
d_proc(struct TRIGGER_DATA* data)
{
	redisReply *reply;
	char rkey[RBUf_LEN];
	char num[RBUf_LEN];
	char buf[RBUf_LEN];
	int re;
	REDIS_CONF *rc;
	REDIS_CONF *rcn;
	int i=0;
	uint32_t N; /* crc-key % mod */
	uint32_t crc;

	/* get redis common configure from RB-TREE */
	rc = get_redis_common_conf(data->dbname, data->tbname);
	if (NULL == rc) {
		//MYLOG("redis common configure[%s.%s.common] not found!",
			//data->dbname, data->tbname);
		return -1;
	}
	/* check redis_key */
	if (rc->redis_key < 0 || rc->redis_key >= data->filednum) {
		MYLOG("invalid redis_key[section: %s.%s.common, redis_key: %d]",
			data->dbname, data->tbname, rc->redis_key);
		return -1;
	}
	/* determine redis key */
	if(NULL == data->row_list[rc->redis_key].data) {
		MYLOG("key is NULL[section: %s.%s.common, redis_key: %d]",
			data->dbname, data->tbname, rc->redis_key);
		return -1;
	}
	/* get value string */
	re = db_value2string(&data->row_list[rc->redis_key], rkey);
	if (re < 0) {
		MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, rkey);
		return -1;
	}
	/* hash function is crc32 */
	crc = mycrc32((uint8_t*)rkey, re);
	if (rc->mod <=0 ) {
		MYLOG("invalid mod[%d]", rc->mod);
		return -1;
	}
	N = crc % rc->mod;
	/* get redis server configure, contain connection */
	rcn = get_redis_server_conf(data->dbname, data->tbname, N);
	if (NULL == rcn) {
		//MYLOG("redis server configure[%s.%s.%u] not found!",
		//	data->dbname, data->tbname, N);
		return -1;
	}
	/* select redis db */
	if (select_db(rcn)) {
		MYLOG("select redis db faild!");
		return -1;
	}

	/* process data */
	if (!strncmp("row", rc->redis_value_type, sizeof("row") - 1)) { /* row data */
		/* implement reconnect after */
		reply = myRedisCommand(rcn, "DEL %s", rkey);
		if (reply->type == REDIS_REPLY_ERROR) {
			MYLOG("myRedisCommand error[%s]", reply->str);
		}
		freeReplyObject(reply);
	} else if (!strncmp("list", rc->redis_value_type, sizeof("list") - 1)){ /* row data */
		/* check redis_value */
		if (rc->redis_value < 0 || rc->redis_value >= data->filednum) {
			MYLOG("invalid redis_value[section: %s.%s.common, redis_key: %d]",
				data->dbname, data->tbname, rc->redis_value);
			return -1;
		}
		re = db_value2string(&data->row_list[rc->redis_value], buf);
		if (re < 0) {
			MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, buf);
			return -1; /*?*/
		}
		/* implement reconnect after */
		reply = myRedisCommand(rcn, "SREM %s %s", rkey, buf);
		if (reply->type == REDIS_REPLY_ERROR) {
			MYLOG("myRedisCommand error[%s]", reply->str);
		}
		freeReplyObject(reply);
	} else { /* exception */
		MYLOG("nonsupport redis value type[%s]", rc->redis_value_type);
		return -1;
	}

	record_position(rcn, data);

	return  0;
}

int 
u_proc(struct TRIGGER_DATA* data) {
	redisReply *reply;
	char rkey[RBUf_LEN];
	char num[RBUf_LEN];
	char buf[RBUf_LEN];
	int re;
	REDIS_CONF *rc;
	REDIS_CONF *rcn;
	int i=0;
	uint32_t N; /* crc-key % mod */
	uint32_t crc;

	/* get redis common configure from RB-TREE */
	rc = get_redis_common_conf(data->dbname, data->tbname);
	if (NULL == rc) {
		//MYLOG("redis common configure[%s.%s.common] not found!",
		//	data->dbname, data->tbname);
		return -1;
	}
	/* check redis_key */
	if (rc->redis_key < 0 || rc->redis_key >= data->filednum) {
		MYLOG("invalid redis_key[section: %s.%s.common, redis_key: %d]",
			data->dbname, data->tbname, rc->redis_key);
		return -1;
	}
	/* determine redis key */
	if(NULL == data->row_list[rc->redis_key].data) {
		MYLOG("key is NULL[section: %s.%s.common, redis_key: %d]",
			data->dbname, data->tbname, rc->redis_key);
		return -1;
	}
	/* get value string */
	re = db_value2string(&data->row_list[rc->redis_key], rkey);
	if (re < 0) {
		MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, rkey);
		return -1;
	}
	/* hash function is crc32 */
	crc = mycrc32((uint8_t*)rkey, re);
	if (rc->mod <=0 ) {
		MYLOG("invalid mod[%d]", rc->mod);
		return -1;
	}
	N = crc % rc->mod;
	/* get redis server configure, contain connection */
	rcn = get_redis_server_conf(data->dbname, data->tbname, N);
	if (NULL == rcn) {
		//MYLOG("redis server configure[%s.%s.%u] not found!",
		//	data->dbname, data->tbname, N);
		return -1;
	}
	/* select redis db */
	if (select_db(rcn)) {
		MYLOG("select redis db faild!");
		return -1;
	}

	/* process data */
	if (!strncmp("row", rc->redis_value_type, sizeof("row") - 1)) { /* row data */
		for(i=0 ; i < data->filednum; i++) {
			if (rc->redis_key == i) { /* skip key */
				continue;
			}
			snprintf(num, RBUf_LEN, "%d", i); /* hset field */
			re = db_value2string(&data->row_list_update[i], buf);
			if (re < 0) {
				MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, buf);
				return -1; /*?*/
			}
			/* implement reconnect after */
			reply = myRedisCommand(rcn, "HSET %s %s %s", rkey, num, buf);
			if (reply->type == REDIS_REPLY_ERROR) {
				MYLOG("myRedisCommand error[%s]", reply->str);
			}
			freeReplyObject(reply);
		}
	} else if (!strncmp("list", rc->redis_value_type, sizeof("list") - 1)){ /* row data */
		/* check redis_value */
		if (rc->redis_value < 0 || rc->redis_value >= data->filednum) {
			MYLOG("invalid redis_value[section: %s.%s.common, redis_key: %d]",
				data->dbname, data->tbname, rc->redis_value);
			return -1;
		}
		re = db_value2string(&data->row_list[rc->redis_value], buf);
		if (re < 0) {
			MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, buf);
			return -1; /*?*/
		}
		/* implement reconnect after */
		reply = myRedisCommand(rcn, "ZREM %s %s", rkey, buf);
		if (reply->type == REDIS_REPLY_ERROR) {
			MYLOG("myRedisCommand error[%s]", reply->str);
		}
		freeReplyObject(reply);

		re = db_value2string(&data->row_list_update[rc->redis_value], buf);
		if (re < 0) {
			MYLOG("db_value2string faild, need buf length is %d + 1 [%s]", -re, buf);
			return -1; /*?*/
		}
		/* implement reconnect after */
		reply = myRedisCommand(rcn, "SADD %s %s", rkey, buf);
		if (reply->type == REDIS_REPLY_ERROR) {
			MYLOG("myRedisCommand error[%s]", reply->str);
		}
		freeReplyObject(reply);
	} else { /* exception */
		MYLOG("nonsupport redis value type[%s]", rc->redis_value_type);
		return -1;
	}

	record_position(rcn, data);

	return  0;
}

