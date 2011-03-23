/*-
 * Copyright (c) 2010-2011 SINA Corporation, All Rights Reserved.
 * Author: Zhao pengcheng <pengcheng2@staff.sina.com.cn>
 * create: 2011-3-18 11:50
 */
#ifndef __DATA_UNIT_H__
#define __DATA_UNIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAME_LEN (1024)
#define MAX_BUF_LEN (2048)
#define RECORED_POSITION_F (2)

/* data unit, from mytrigger to so */
typedef struct field_data {
	void *d;
	/* enumeration enum_field_types */
	int d_type;
	int d_len;
} FIELD_DATA;

typedef struct row_data {
	/* enumeration Log_event_type */
	int event_type;

	/* binlog position */
	char log_name[MAX_NAME_LEN + 1];
	unsigned long long log_pos;
	int stmt_index;

	/* databases name and table name */
	char db_name[MAX_NAME_LEN + 1];
	char tb_name[MAX_NAME_LEN + 1];

	/* row fields */
	int field_list_len;
	/* new row data */
	FIELD_DATA* field_list_new;
	/* old row data */
	FIELD_DATA* field_list_old;

	/* statement end flag */
	int stmt_end_f;
} ROW_DATA;

#ifdef __cplusplus
} // extern "C"
#endif
#endif /* #ifndef __DATA_UNIT_H__ */

