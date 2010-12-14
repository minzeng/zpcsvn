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
char list_key[64];
char buf[64];

extern char *redis_ip;
extern int redis_port;

int
init_proc(char *err_msg) {
	c = redisConnect(redis_ip, redis_port);
	if (c->err) {
		LOG(err_msg, "Connection error: %s\n", c->errstr);
		return -1;
	}
	return 0;
}

int
deinit_proc(char *err_msg) {
	redisFree(c);
	return 0;
}

int i_proc(struct TRIGGER_DATA* data)
{
	int i=0;
	printf("^^^^^^^^^^^Database   is: %s^^^^^^^^^^^^\n",data->dbname);
	printf("^^^^^^^^^^^Table name is: %s^^^^^^^^^^^^\n",data->tbname);
	printf("^^^^^^^^^^^Filed num  is: %d^^^^^^^^^^^^\n",data->filednum);

	/* process attention only */
	if (memcmp(data->dbname, "attention", sizeof("attention"))) {
		return -1;
	}

/*	if (NULL == c) {
		c = redisConnect((char*)"127.0.0.1", 9736);
		if (c->err) {
			printf("Connection error: %s\n", c->errstr);
			exit(1);
		}
	}*/

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
				if (0 == i) {
					snprintf(list_key, 64, "%ju", *(intmax_t*)data->row_list[i].data);
				} else {
					snprintf(buf, 64, "%ju", *(intmax_t*)data->row_list[i].data);
					reply = redisCommand(c,"LPUSH %s %s", list_key, buf);
					freeReplyObject(reply);
				}
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));
				break;
			case MYSQL_TYPE_TIMESTAMP:
				if(data->row_list[i].length>0){
				printf("^^^^^^^^^^^Data%d  is: %d^^^^^^^^^^^^\n",i+1,*((long long *)data->row_list[i].data));
				}else{
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
	//redisFree(c);
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

