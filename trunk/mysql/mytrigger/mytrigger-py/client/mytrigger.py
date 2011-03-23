#!/usr/local/bin/python
# Filename: mytrigger.py
#author: zhaopengcheng(pengcheng2@staff.sina.com.cn)

import os
import sys
import redis
import json

redis_h = None

# return value: less than zero is error, otherwise success
def init():
	global redis_h
	print "init"
	redis_h = redis.Redis(host='127.0.0.1', port=7701)
	return 0

# return value: less than zero is error, otherwise success
def deinit():
	print "deinit"
	return 0;

# input row_data is json string
# return value: less than zero is error,
#				2 is mytrigger recored position,
#				otherwise success.
def process(row_data):
	print "data: " + row_data
	data = json.loads(row_data)
	print data
	redis_h.set(data['info']['0.new'], row_data)
	return 2;
