#include <stdio.h>
#include "mystruct.h"
/* redis client lib */
#include "hiredis.h"
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

/* connect redis */
unsigned int j;
static redisContext *c = NULL; 
redisReply *reply;

#define RKEY_LEN 128
#define RBUf_LEN 1024
char rkey[RKEY_LEN];
char num[RKEY_LEN];
char buf[RBUf_LEN];

extern char *redis_ip;
extern int redis_port;

int
init_proc(char *err_msg) {
	c = redisConnect(redis_ip, redis_port);
	if (c->err) {
		LOG(err_msg, "Connection error[%s:%d]: %s\n", redis_ip, redis_port, c->errstr);
		return -1;
	}
	return 0;
}

int
deinit_proc(char *err_msg) {
	redisFree(c);
	return 0;
}

#define PROC_DB "user"
#define PROC_TABLE "user_base"

int i_proc(struct TRIGGER_DATA* data)
{
	int i=0;
	printf("^^^^^^^^^^^Database   is: %s^^^^^^^^^^^^\n",data->dbname);
	printf("^^^^^^^^^^^Table name is: %s^^^^^^^^^^^^\n",data->tbname);
	printf("^^^^^^^^^^^Filed num  is: %d^^^^^^^^^^^^\n",data->filednum);


	/* select db */ /* select table */
	if (memcmp(data->dbname, PROC_DB, sizeof(PROC_DB))) {
		return -1;
	}
	if (memcmp(data->tbname, PROC_TABLE, sizeof(PROC_TABLE))) {
		return -1;	
	}

	for(i=0 ; i < data->filednum; i++) {
		if(NULL == data->row_list[i].data) {
			printf("^^^^^^^^^^^NO DATA^^^^^^^^^^^^^%d^^^^^^^^^^^^^^^^^^^^^^^\n",data->row_list[i].type);
			continue;
		}
		snprintf(num, RKEY_LEN, "%d", i);
		switch(data->row_list[i].type) {
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_LONG:
			snprintf(buf, RBUf_LEN, "%d", *(int*)data->row_list[i].data);
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((int *)data->row_list[i].data));
			break;
		case MYSQL_TYPE_FLOAT:
			snprintf(buf, RBUf_LEN, "%-20g", *((float *)data->row_list[i].data));
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %f^^^^^^^^^^^^\n",i+1,*((float *)data->row_list[i].data));
			break;
		case MYSQL_TYPE_DOUBLE:
			snprintf(buf, RBUf_LEN, "%-.20g", *((double *)data->row_list[i].data));
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %f^^^^^^^^^^^^\n",i+1,*((double *)data->row_list[i].data));
			break;
		case MYSQL_TYPE_VARCHAR:
			snprintf(buf, RBUf_LEN, "%s", data->row_list[i].data);
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
			break;
		case MYSQL_TYPE_BLOB:
			snprintf(buf, RBUf_LEN, "%s", data->row_list[i].data);
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
			break;
		case MYSQL_TYPE_DATETIME:
			snprintf(buf, RBUf_LEN, "%s", data->row_list[i].data);
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
			break;
		case MYSQL_TYPE_LONGLONG:
			if (0 == i) {
				snprintf(rkey, RKEY_LEN, "%ju", *(intmax_t*)data->row_list[i].data);
			} else {
				snprintf(buf, RBUf_LEN, "%ju", *(intmax_t*)data->row_list[i].data);
				reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
				freeReplyObject(reply);
			}
			printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));
			break;
		case MYSQL_TYPE_TIMESTAMP:
			if(data->row_list[i].length>0){
				snprintf(buf, RBUf_LEN, "%ju", *(intmax_t*)data->row_list[i].data);
				reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
				freeReplyObject(reply);
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));
			}else{
				snprintf(buf, RBUf_LEN, "%s", data->row_list[i].data);
				reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
				freeReplyObject(reply);
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
			}
			break;
		case MYSQL_TYPE_STRING:
			snprintf(buf, RBUf_LEN, "%s", data->row_list[i].data);
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
			break;
		case MYSQL_TYPE_TINY:
			snprintf(buf, RBUf_LEN, "%d", *(int*)data->row_list[i].data);
			reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
			freeReplyObject(reply);
			printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((int *)data->row_list[i].data));
			break;
		case MYSQL_TYPE_DATE:
			if(data->row_list[i].length>0){
				snprintf(buf, RBUf_LEN, "%ju", *(intmax_t*)data->row_list[i].data);
				reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
				freeReplyObject(reply);
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));}
			else{
				snprintf(buf, RBUf_LEN, "%s", data->row_list[i].data);
				reply = redisCommand(c,"HSET %s %s %s", rkey, num, buf);
				freeReplyObject(reply);
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
			}
			break;
		default:
			printf("^^^^^^^^^^^ERR^^^^^^^^^^^^^%d^^^^^^^^^^^^^^^^^^^^^^^\n",data->row_list[i].type);
			break;
		}
	}
	printf("^^^^^^^^^^^^^^^^^^^end^^^^^^^^^^^^^^^^^^\n");
	return data->filednum;
}

int d_proc(struct TRIGGER_DATA* data)
{
		int i=0;
	printf("^^^^^^^^^^^Database   is: %s^^^^^^^^^^^^\n",data->dbname);
	printf("^^^^^^^^^^^Table name is: %s^^^^^^^^^^^^\n",data->tbname);
	printf("^^^^^^^^^^^Filed num  is: %d^^^^^^^^^^^^\n",data->filednum);
	for(i=0;i<data->filednum;i++)
	{
			if(data->row_list[i].data==NULL)
				{
				printf("^^^^^^^^^^^NO DATA^^^^^^^^^^^^^%d^^^^^^^^^^^^^^^^^^^^^^^\n",data->row_list[i].type);
				continue;
				}
			switch(data->row_list[i].type)
			{
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((int *)data->row_list[i].data));
				break;
			case MYSQL_TYPE_FLOAT:
				printf("^^^^^^^^^^^Data%d  is: %f^^^^^^^^^^^^\n",i+1,*((float *)data->row_list[i].data));
				break;
			case MYSQL_TYPE_DOUBLE:
				printf("^^^^^^^^^^^Data%d  is: %f^^^^^^^^^^^^\n",i+1,*((double *)data->row_list[i].data));
				break;
			case MYSQL_TYPE_VARCHAR:
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
				break;
			case MYSQL_TYPE_BLOB:
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
				break;
			case MYSQL_TYPE_DATETIME:
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
				break;
			case MYSQL_TYPE_LONGLONG:
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));
				break;
			case MYSQL_TYPE_TIMESTAMP:
				if(data->row_list[i].length>0){
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));}
				else{
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);	
					}
				break;
			case MYSQL_TYPE_STRING:
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);
				break;
			case MYSQL_TYPE_TINY:
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((int *)data->row_list[i].data));
				break;
			case MYSQL_TYPE_DATE:
				if(data->row_list[i].length>0){
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));}
				else{
				printf("^^^^^^^^^^^Data%d  is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data);	
					}
				break;
			default:
				printf("^^^^^^^^^^^ERR^^^^^^^^^^^^^%d^^^^^^^^^^^^^^^^^^^^^^^\n",data->row_list[i].type);
				break;
			}
	}
	printf("^^^^^^^^^^^^^^^^^^^end^^^^^^^^^^^^^^^^^^\n");
	return data->filednum;
}

int u_proc(struct TRIGGER_DATA* data)
{
		
		int i=0;
		printf("^^^^^^^^^^^Database   is: %s^^^^^^^^^^^^\n",data->dbname);
		printf("^^^^^^^^^^^Table name is: %s^^^^^^^^^^^^\n",data->tbname);
		printf("^^^^^^^^^^^Filed num  is: %d^^^^^^^^^^^^\n",data->filednum);
		for(i=0;i<data->filednum;i++)
		{
			if(data->row_list[i].data==NULL||data->row_list_update[i].data==NULL)
				{
				printf("^^^^^^^^^^^NO DATA^^^^^^^^^^^^^%d^^^^^^^^^^^^^^^^^^^^^^^\n",data->row_list[i].type);
				continue;
				}
			switch(data->row_list[i].type)
			{
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
				printf("^^^^^^^^^^^Data%d old is: %d  new is: %d^^^^^^^^^^^^\n",i+1,*((int *)data->row_list[i].data),*((int *)data->row_list_update[i].data));
				break;
			case MYSQL_TYPE_FLOAT:
				printf("^^^^^^^^^^^Data%d old is: %f  new is: %f^^^^^^^^^^^^\n",i+1,*((float *)data->row_list[i].data),*((float *)data->row_list_update[i].data));
				break;
			case MYSQL_TYPE_DOUBLE:
				printf("^^^^^^^^^^^Data%d old is: %f  new is: %f^^^^^^^^^^^^\n",i+1,*((double *)data->row_list[i].data),*((double *)data->row_list_update[i].data));
				break;
			case MYSQL_TYPE_VARCHAR:
				printf("^^^^^^^^^^^Data%d old is: %s  new is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data,data->row_list_update[i].data);
				break;
			case MYSQL_TYPE_BLOB:
				printf("^^^^^^^^^^^Data%d old is: %s  new is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data,data->row_list_update[i].data);
				break;
			case MYSQL_TYPE_DATETIME:
				printf("^^^^^^^^^^^Data%d old is: %s  new is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data,data->row_list_update[i].data);
				break;
			case MYSQL_TYPE_LONGLONG:
				printf("^^^^^^^^^^^Data%d old is: %d  new is: %d^^^^^^^^a^^^^\n",i+1,*((long long *)data->row_list[i].data),*((long long *)data->row_list_update[i].data));
				break;
			case MYSQL_TYPE_TIMESTAMP:
				printf("^^^^^^^^^^^Data%d old is: %d  new is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data),*((long long *)data->row_list_update[i].data));
				break;
			case MYSQL_TYPE_STRING:
				printf("^^^^^^^^^^^Data%d old is: %s  new is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data,data->row_list_update[i].data);
				break;
			case MYSQL_TYPE_TINY:
				printf("^^^^^^^^^^^Data%d old is: %d  new is: %d^^^^^^^^^^^^\n",i+1,*((int *)data->row_list[i].data),*((int *)data->row_list_update[i].data));
				break;
			case MYSQL_TYPE_DATE:
				if(data->row_list[i].length>0){
				printf("^^^^^^^^^^^Data%d old is: %d  new is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data),*((long long *)data->row_list_update[i].data));}
				else{
				printf("^^^^^^^^^^^Data%d old is: %s  new is: %s^^^^^^^^^^^^\n",i+1,data->row_list[i].data,data->row_list_update[i].data);
					}
				break;
			default:
				printf("^^^^^^^^^^^ERR^^^^^^^^^^^^^%d^^^^^^^^^^^^^^^^^^^^^^^\n",data->row_list[i].type);
				break;
			}
		
		}
		printf("^^^^^^^^^^^^^^^^^^^end^^^^^^^^^^^^^^^^^^\n");
		return data->filednum;
}

