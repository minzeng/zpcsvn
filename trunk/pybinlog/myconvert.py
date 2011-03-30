#!/usr/bin/python

import	struct

def ConvertDecimalToNumber(buffer):
	length = len(buffer)
	format = '='
        for i in range(length):
                format = format + 'B'
	
	mytuple = struct.unpack(format,buffer)
	mylist  = list(mytuple)
	mylist.reverse()	

	result = 0
        for i in range(length):
                result  +=   mylist[i]*(1 << (8*i))

        return result

def ConvertStringToUnInt(buffer):
        length = len(buffer)

        format = '='
        for i in range(length):
                format = format + 'B'

        mytuple = struct.unpack(format,buffer)

        result = 0
        for i in range(length):
                result  +=   mytuple[i]*(1 << (8*i))

        return result

def ConvertStringToInt(buffer):
        length = len(buffer)

        format = '='
        for i in range(length):
                format = format + 'B'

        mytuple = list(struct.unpack(format,buffer))

        mytuple_length = len(mytuple)

        enditem = mytuple[mytuple_length - 1]
        flag = enditem & 0x80
        mytuple[mytuple_length-1] = enditem & 0x7F

        result = 0
        for i in range(length):
                result  +=   mytuple[i]*(1 << (8*i))


        if flag == 0:
                return result
        else:
                return (result - (1 << (8*(length)-1)))


def ConvertIntToString(n,length):
	value = n

	mid = value & 0xFF
	buf = struct.pack('=B',mid)
	value = value >> 8	

	ncount = 1
	while ncount != length:
		mid = value & 0xFF
		buf = buf + struct.pack('=B',mid)	
		value = value >> 8 

		ncount = ncount + 1

	return buf
