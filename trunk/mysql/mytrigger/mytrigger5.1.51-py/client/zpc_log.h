/*-
 * Copyright (c) 2010-2011 SINA Corporation, All Rights Reserved.
 * Author: Zhao pengcheng <pengcheng2@staff.sina.com.cn>
 * create: 2011-3-18 11:50
 */
#ifndef __ZPCLOG_H__
#define __ZPCLOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Log levels */
#define ZPC_DEBUG 0
#define ZPC_VERBOSE 1
#define ZPC_NOTICE 2
#define ZPC_WARNING 3
#define ZPC_ERROR 4

void zpc_log(int level, const char *fmt, ...);

#define MYLOG(level, fmt, arg...) \
	zpc_log(level, "%s[%d]: "#fmt, __func__, __LINE__, ##arg);

#ifdef __cplusplus
} // extern "C"
#endif
#endif /* __ZPCLOG_H__ */
