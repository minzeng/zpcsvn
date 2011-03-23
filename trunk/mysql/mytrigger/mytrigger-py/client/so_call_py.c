/*-
 * Copyright (c) 2010-2011 SINA Corporation, All Rights Reserved.
 * Author: Zhao pengcheng <pengcheng2@staff.sina.com.cn>
 * create: 2011-3-20 18:59
 */

#include <Python.h>
#include <json/json.h>

#include "mytrigger_so.h"
#include "zpc_log.h"


#define INIT_FUNC   "init"
#define DEINIT_FUNC "deinit"
#define PROC_FUNC   "process"

static const char *SCRIPT;

static PyObject *pModule;
static PyObject *pFunc_Proc;

int init_proc(char *pymod) {
	PyObject *pName;
	PyObject *pFunc;
	PyObject *pValue;
	long re = 0;

	SCRIPT = pymod;
	Py_Initialize();
    PyRun_SimpleString( "import sys\n"
		"sys.path = ['']+sys.path\n" );

	pName = PyString_FromString(SCRIPT);
	if (NULL == pName) {
		MYLOG(ZPC_ERROR, "faild");
		PyErr_Print();
		Py_DECREF(pName);
		return -1;
	}
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);
	if (NULL == pModule) {
		MYLOG(ZPC_ERROR, "faild");
		PyErr_Print();
		return -1;
	}
	pFunc = PyObject_GetAttrString(pModule, INIT_FUNC);
	if (!pFunc || !PyCallable_Check(pFunc)) {
		MYLOG(ZPC_ERROR, "faild");
		PyErr_Print();
		return -1;
	}
	pValue = PyObject_CallObject(pFunc, NULL);
	if (NULL != pValue) {
		re = PyInt_AsLong(pValue);
		Py_DECREF(pValue);
	}
	Py_DECREF(pFunc);

	pFunc_Proc = PyObject_GetAttrString(pModule, PROC_FUNC);
	if (!pFunc_Proc || !PyCallable_Check(pFunc_Proc)) {
		MYLOG(ZPC_ERROR, "faild");
		PyErr_Print();
		return -1;
	}
	return re;
}

/* mysql data type */
enum enum_field_types {
	MYSQL_TYPE_DECIMAL,
	MYSQL_TYPE_TINY,
	MYSQL_TYPE_SHORT,
	MYSQL_TYPE_LONG,
	MYSQL_TYPE_FLOAT,
	MYSQL_TYPE_DOUBLE,
	MYSQL_TYPE_NULL,
	MYSQL_TYPE_TIMESTAMP,
	MYSQL_TYPE_LONGLONG,
	MYSQL_TYPE_INT24,
	MYSQL_TYPE_DATE,
	MYSQL_TYPE_TIME,
	MYSQL_TYPE_DATETIME,
	MYSQL_TYPE_YEAR,
	MYSQL_TYPE_NEWDATE,
	MYSQL_TYPE_VARCHAR,
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

enum Log_event_type {
	
	//These event numbers are used from 5.1.16 and forward

	WRITE_ROWS_EVENT = 23,
	UPDATE_ROWS_EVENT = 24,
	DELETE_ROWS_EVENT = 25
};

unsigned long long num_val;
int db_value2string_unsigned(FIELD_DATA *md, char *buf) {
	int re;

	if (NULL == md->d) {
		re = snprintf(buf, MAX_BUF_LEN,
			"NO DATATYPE[%d]", md->d_type);
		return -1;
	}
	switch(md->d_type) {
		case MYSQL_TYPE_SHORT:
			re = snprintf(buf, MAX_BUF_LEN, "%u", *((unsigned short *)md->d));
			num_val = *((unsigned short *)md->d);
		case MYSQL_TYPE_LONG:
			re = snprintf(buf, MAX_BUF_LEN, "%u", *((int *)md->d));
			num_val = *((int *)md->d);
			break;
		case MYSQL_TYPE_FLOAT:
			re = snprintf(buf, MAX_BUF_LEN, "%f", *((float *)md->d));
			break;
		case MYSQL_TYPE_DOUBLE:
			re = snprintf(buf, MAX_BUF_LEN, "%f", *((double *)md->d));
			break;
		case MYSQL_TYPE_VARCHAR:
			re = snprintf(buf, MAX_BUF_LEN, "%s", (char*)md->d);
			break;
		case MYSQL_TYPE_BLOB:
			re = snprintf(buf, MAX_BUF_LEN, "%s", (char*)md->d);
			break;
		case MYSQL_TYPE_DATETIME:
			re = snprintf(buf, MAX_BUF_LEN, "%s", (char*)md->d);
			break;
		case MYSQL_TYPE_LONGLONG:
			re = snprintf(buf, MAX_BUF_LEN, "%llu", 
				*((unsigned long long *)md->d));
			num_val = *((unsigned long long *)md->d);
			break;
		case MYSQL_TYPE_TIMESTAMP:
			if(md->d_len > 0) {
				re = snprintf(buf, MAX_BUF_LEN, "%u",
					*((unsigned int *)md->d));
				num_val = *((unsigned int *)md->d);
			} else {
				re = snprintf(buf, MAX_BUF_LEN, "%s", (char*)md->d);
			}
			break;
		case MYSQL_TYPE_STRING:
			re = snprintf(buf, MAX_BUF_LEN, "%s", (char*)md->d);
			break;
		case MYSQL_TYPE_TINY:
			re = snprintf(buf, MAX_BUF_LEN, "%u", *((unsigned char *)md->d));
			num_val = *((unsigned char *)md->d);
			break;
		case MYSQL_TYPE_DATE:
			if(md->d_len > 0) {
				re = snprintf(buf, MAX_BUF_LEN, "%llu", *((long long *)md->d));
			} else {
				re = snprintf(buf, MAX_BUF_LEN, "%s", (char*)md->d);
			}
			break;
		case MYSQL_TYPE_NULL:
			*buf = 0;
			break;
		default:
			re = snprintf(buf, MAX_BUF_LEN, "unknow type[%d]", md->d_type);
			return -1;
	}
	if (re >= MAX_BUF_LEN) {
		return -re;
	} else {
		return re;
	}
}

char *act_str(int act_num) {
	switch (act_num) {
		case WRITE_ROWS_EVENT:
			return (char*)"insert";
		case UPDATE_ROWS_EVENT:
			return (char*)"update";
		case DELETE_ROWS_EVENT:
			return (char*)"delete";
		default:
			return (char*)"no-act";
	}
}

char* row_data_to_json_str(ROW_DATA* rd) {
	int i = 0;
	char *re = NULL;
	json_object *row, *all;
	FIELD_DATA* data;
	char field_name[MAX_NAME_LEN + 1] = {0};
	char value_buf[MAX_BUF_LEN + 1] = {0};
	char pos_buf[MAX_BUF_LEN + 1] = {0};

	if (NULL == rd) {
		MYLOG(ZPC_ERROR, "input is null");
		return NULL;
	}
	all = json_object_new_object();
	row = json_object_new_object();
	json_object_object_add(all, "db", json_object_new_string(rd->db_name));
	json_object_object_add(all, "tbl", json_object_new_string(rd->tb_name));
	snprintf(pos_buf, MAX_BUF_LEN, "%s/%lu/%d",
		rd->log_name, rd->log_pos, rd->stmt_index);
	json_object_object_add(all, "pos", json_object_new_string(pos_buf));
	json_object_object_add(all, "act",
		json_object_new_string(act_str(rd->event_type)));
	if (rd->field_list_new) {
		data = rd->field_list_new;
		for (i = 0; i < rd->field_list_len; i++) {
			snprintf(field_name, MAX_NAME_LEN, "%d.new", i);
			db_value2string_unsigned(&data[i], value_buf);
			json_object_object_add(row, field_name,
				json_object_new_string(value_buf));
		}
	}
	if (rd->field_list_old && rd->event_type == UPDATE_ROWS_EVENT) {
		data = rd->field_list_old;
		for (i = 0; i < rd->field_list_len; i++) {
			snprintf(field_name, MAX_NAME_LEN, "%d.old", i);
			db_value2string_unsigned(&data[i], value_buf);
			json_object_object_add(row, field_name,
				json_object_new_string(value_buf));
		}
	}
	json_object_object_add(all, "info", row);
	re = strdup(json_object_to_json_string(all));
	json_object_put(all);
	return re;
}

int process(ROW_DATA* rd) {
	PyObject *pArgs;
	PyObject *pValue;
	long re = 0;
	char *data_json_str = NULL;
	
	pArgs = PyTuple_New(1);
	data_json_str = row_data_to_json_str(rd);
	if (NULL == data_json_str) {
		MYLOG(ZPC_ERROR, "row_data_to_json_str faild");
		return -1;
	}
	pValue = PyString_FromString(data_json_str);
	if (!pValue) {
		free(data_json_str);
		Py_DECREF(pArgs);
		MYLOG(ZPC_ERROR, "faild");
		PyErr_Print();
		return -1;
	}
	PyTuple_SetItem(pArgs, 0, pValue);
	free(data_json_str);

	pValue = PyObject_CallObject(pFunc_Proc, pArgs);
	Py_DECREF(pArgs);
	if (NULL != pValue) {
		re = PyInt_AsLong(pValue);
		Py_DECREF(pValue);
		return re;
	} else {
		MYLOG(ZPC_ERROR, "faild");
		PyErr_Print();
		return -1;
	}
}

int deinit_proc() {
	PyObject *pFunc;
	PyObject *pValue;
	long re = 0;

	pFunc = PyObject_GetAttrString(pModule, DEINIT_FUNC);
	if (!pFunc || !PyCallable_Check(pFunc)) {
		MYLOG(ZPC_ERROR, "faild");
		PyErr_Print();
		return -1;
	}
	pValue = PyObject_CallObject(pFunc, NULL);
	if (NULL != pValue) {
		re = PyInt_AsLong(pValue);
		Py_DECREF(pValue);
	}
	Py_DECREF(pFunc);
	Py_XDECREF(pFunc_Proc);
	Py_DECREF(pModule);
	Py_Finalize();
	return re;
}

