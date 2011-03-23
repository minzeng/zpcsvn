/*-
 * Copyright (c) 2010-2011 SINA Corporation, All Rights Reserved.
 * Author: Zhao pengcheng <pengcheng2@staff.sina.com.cn>
 * create: 2011-3-18 11:50
 */
#ifndef __MYTRIGGERSO_H__
#define __MYTRIGGERSO_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "row_data.h"
#include "zpc_log.h"

/* SO API */
int init_proc(char* pymod);
int deinit_proc();
int process(ROW_DATA*);

#endif /* __MYTRIGGERSO_H__ END*/

