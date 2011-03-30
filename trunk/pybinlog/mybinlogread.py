#!/usr/bin/python
import os
import struct
import sys
import string
import json
import mytypes
import mydecimal
import myconvert
from mydecimal import decimal_bin_size
from mydecimal import binary2my_decimal
from mydecimal import DIG_PER_DEC1

from myconvert import ConvertStringToUnInt
from myconvert import ConvertStringToInt
from mytable   import TableMap

tableMap = TableMap()

EVENT_HEADER_LENGTH = 19


def ROUND_UP(X):
	 return (((X)+DIG_PER_DEC1-1)/DIG_PER_DEC1)

"""
def GetDescEvent(fileobject):

	buffer = fileobject.read(76)

	if len(buffer) != 76:
		return -1

	start_event_tuple = struct.unpack("=ibiiihh50sib",buffer)
	start_event_dict = {}
	start_event_dict['timestamp'] = start_event_tuple[0];
	start_event_dict['type_code'] = start_event_tuple[1];
	start_event_dict['server_id'] = start_event_tuple[2];
	start_event_dict['event_length'] = start_event_tuple[3];
	start_event_dict['next_position'] = start_event_tuple[4];
	start_event_dict['flags'] = start_event_tuple[5];
	start_event_dict['binlog_version'] = '%s'%(start_event_tuple[6]);
	start_event_dict['server_version'] = start_event_tuple[7];
	start_event_dict['create_timestamp'] = start_event_tuple[8];
	start_event_dict['header_length'] = start_event_tuple[9];

	nlength = start_event_dict['event_length'] - 76
	
	buffer = fileobject.read(27);
	start_event_dict['post_header'] = struct.unpack('=bbbbbbbbbbbbbbbbbbbbbbbbbbb',buffer)	

	return start_event_dict;


def GetNextEvent(fileobject,header_length):

#event header
	buffer = fileobject.read(19)
	
	if len(buffer) != 19:
		return -1	

	event_header_tuple = struct.unpack("=ibiiih",buffer)		
	event_dict = {}
        event_dict['timestamp'] = event_header_tuple[0];
        event_dict['type_code'] = event_header_tuple[1];
        event_dict['server_id'] = event_header_tuple[2];
        event_dict['event_length'] = event_header_tuple[3];
        event_dict['next_position'] = event_header_tuple[4];
        event_dict['flags'] = event_header_tuple[5];
	data_length = event_dict['event_length'] - header_length

#variable part	
	event_dict['data'] = fileobject.read(data_length)	

	if len(event_dict['data']) != data_length:
		return -1 


#store all event data 
#	fileobject.seek(0-event_dict['event_length'],1)
#	event_dict['buffer'] = fileobject.read(event_dict['event_length'])
	
	return event_dict

"""

def DecodePackedInt(buffer):
	pos = ConvertStringToUnInt(buffer[0:1])
	if pos < 251:
		return buffer[1:len(buffer)],pos
	elif pos == 251:
		return buffer,0
	elif pos == 252:
		return buffer[4:len(buffer)],ConvertStringToUnInt(buffer[1:3]) 
	elif pos == 253:
		return buffer[5:len(buffer)],ConvertStringToUnInt(buffer[1:4])
	else:
		return buffer[10:len(buffer)],ConvertStringToUnInt(buffer[1:4])

def DecodeRowImageItem(buffer,type,meta):
	length = 0
	if type == mytypes.MYSQL_TYPE_STRING:	#equal == log_event_print_value
		if meta >= 256:
			byte0 = meta >> 8
			byte1 = meta & 0xFF 
			if (byte0 & 0x30) != 0x30:
				length = byte1 | (((byte0 & 0x30) ^ 0x30) << 4)	
				type= byte0 | 0x30
			else:
          			length= meta & 0xFF;			
				
		else:
			length = meta


	if type == 0:	#MYSQL_TYPE_DECIMAL 
		return
	elif type ==1:	#MYSQL_TYPE_TINY,               
		value = ConvertStringToInt(buffer[0:1])
		return buffer[1:len(buffer)],value,'TINYINT' 
	elif type == mytypes.MYSQL_TYPE_SHORT:  
		value = ConvertStringToInt(buffer[0:2])
		return buffer[2:len(buffer)],value,'SHORTINT'
	elif type == mytypes.MYSQL_TYPE_LONG:   
		value = ConvertStringToInt(buffer[0:4]) 
		return buffer[4:len(buffer)],value,'INT'
	elif type == mytypes.MYSQL_TYPE_FLOAT:
		value = struct.unpack('=f',buffer[0:4])
		return buffer[4:len(buffer)],value,'FLOAT' 
	elif type == mytypes.MYSQL_TYPE_DOUBLE:
		value = struct.unpack('=d',buffer[0:8])
		return buffer[8:len(buffer)],value ,'DOUBLE'
	elif type == 6: #MYSQL_TYPE_NULL,
		return
	elif type == mytypes.MYSQL_TYPE_TIMESTAMP:
		value = ConvertStringToInt(buffer[0:4])		
		return buffer[4:len(buffer)],value,'TIMESTAMP'
	elif type == mytypes.MYSQL_TYPE_LONGLONG:
		longlong = ConvertStringToInt(buffer[0:8])
		return buffer[8:len(buffer)],longlong,'LONGINT'
	elif type == mytypes.MYSQL_TYPE_INT24:
		value = ConvertStringToInt(buffer[0:3])
		return buffer[3:len(buffer)],value,'MEDIUMINT'
	elif type == mytypes.MYSQL_TYPE_DATE:
		i32 = ConvertStringToInt(buffer[0:3])
		value =  "%04d:%02d:%02d"%((i32 / (16L * 32L)), (i32 / 32L % 16L), (i32 % 32L))
		return buffer[3:len(buffer)],value,'DATE'
	elif type == mytypes.MYSQL_TYPE_TIME:
		i32= ConvertStringToInt(buffer[0:3])
		value = "%02d:%02d:%02d"%(i32 / 10000, (i32 % 10000) / 100, i32 % 100)
		return buffer[3:len(buffer)],value,'TIME'
	elif type == mytypes.MYSQL_TYPE_DATETIME:
		i64 = ConvertStringToInt(buffer[0:8])
		d= i64 / 1000000
      		t= i64 % 1000000
		value = "%04d-%02d-%02d %02d:%02d:%02d"%( d / 10000, (d % 10000) / 100, d % 100,t / 10000, (t % 10000) / 100, t % 100)
		return buffer[8:len(buffer)],value,'DATETIME'
	elif type == mytypes.MYSQL_TYPE_YEAR:
		i32 = ConvertStringToInt(buffer[0:1])
		value = "%04d"%(i32+ 1900) 
		return buffer[1:len(buffer)],value,'YEAR'

	elif type == mytypes.MYSQL_TYPE_NEWDATE:
		i32 = ConvertStringToInt(buffer[0:3])
		tmp = [0,0,0,0,0,0,0,0,0,0,0]	#initialize tmp
		tmp[10] = 0;
		part = i32 & 31
		tmp[9] = part%10
		tmp[8] = part/10
		tmp[7] = ':'
		part = (i32 >> 5 & 15)
		tmp[6] = part%10
		tmp[5] = part/10
		tmp[4] = ':'
		part = i32 >> 9
		tmp[3] = part%10
		part = part/10
		tmp[2] = part%10
		part = part/10
		tmp[1] = part%10
		part = part/10
		tmp[0] = part 

		newdate = '%d%d%d%d:%d%d:%d%d'%(tmp[0],tmp[1],tmp[2],tmp[3],tmp[5],tmp[6],tmp[8],tmp[9])		

		return buffer[3:len(buffer)],newdate,'NEWDATE'

	elif type == mytypes.MYSQL_TYPE_BIT:		#Warning
		 # Meta-data: bit_len, bytes_in_rec, 2 bytes 
      		nbits= ((meta >> 8) * 8) + (meta & 0xFF);
      		length= (nbits + 7) / 8;
		return buffer[length:len(buffer)],buffer[0:length],'BIT' 

	elif type == mytypes.MYSQL_TYPE_NEWDECIMAL:
		precision = meta >> 8
		decimals = meta & 0xFF
		bin_size = decimal_bin_size(precision,decimals)
		to = binary2my_decimal(buffer,precision,decimals)				
		end= ROUND_UP(to['frac']) + ROUND_UP(to['intg'])-1
		tmp = ''
		for i in range(end-1):
			tmp = '%s%d'%(tmp,to['buf'][i])
			i= i+1
		result = '%s.%d'%(tmp,to['buf'][i])
		return buffer[bin_size:len(buffer)],result,'NEWDECIMAL'

	elif type == mytypes.MYSQL_TYPE_ENUM:
		bit = meta & 0xFF
		if bit == 1:
			value = ConvertStringToInt(buffer[0:1])
			return buffer[1:len(buffer)],value,'ENUM'
		elif bit == 2:
			value = ConvertStringToInt(buffer[0:2])
			return buffer[2:len(buffer)],value,'ENUM'
		else:
			print 'Unknown ENUM packlen'
			return 0

	elif type == mytypes.MYSQL_TYPE_SET:
		length = meta & 0xFF
		value = buffer[0:length] 
		return buffer[length:len(buffer)],value,'SET'

	elif type == 249: #MYSQL_TYPE_TINY_BLOB=249,
		return
	elif type == 250: #MYSQL_TYPE_MEDIUM_BLOB=250,
		return
	elif type == 251: #MYSQL_TYPE_LONG_BLOB=251,
		return
	elif type == 252: #MYSQL_TYPE_BLOB=252,
		if meta == 1:
			length = ConvertStringToInt(buffer[0:1])
                	return buffer[length+1:len(buffer)],buffer[1:1+length],'TINYBLOB/TINYTEXT'
		elif meta == 2:
			length = ConvertStringToInt(buffer[0:2])
			return buffer[length+2:len(buffer)],buffer[2:2+length],'BLOB/TEXT'
		elif meta == 3:
			length = ConvertStringToInt(buffer[0:3]) #first length,then content
			return buffer[length+3:len(buffer)],buffer[3:3+length],'MEDIUMBLOB/MEDIUMTEXT'
		elif meta == 4:
			length = ConvertStringToInt(buffer[0:4])
			
                        return buffer[length+4:len(buffer)],buffer[4:4+length],'LONGBLOB/LONGTEXT'
		else:
			print '!! Unknown BLOB'
			sys.exit
		return 
	elif type in (mytypes.MYSQL_TYPE_VAR_STRING,mytypes.MYSQL_TYPE_VARCHAR):
		length = meta
		if length < 256:
			length = ConvertStringToInt(buffer[0:1])
			return buffer[length+1:len(buffer)],buffer[1:1+length],('VARSTRING(length=%d)'%length)
		else:
			length = ConvertStringToInt(buffer[0:2])
			return buffer[length+2:len(buffer)],buffer[2:2+length],('VARSTRING(length=%d)'%length)
	elif type == mytypes.MYSQL_TYPE_STRING:
		if length < 256:
			length = ConvertStringToInt(buffer[0:1])
			return buffer[length+1:len(buffer)],buffer[1:1+length],('STRING(length=%d'%length)
		else:
			length = ConvertStringToInt(buffer[0:2])
			return buffer[length+3:len(buffer)],buffer[2:2+length],('STRING(length=%d'%length)
	else:
		print 'Not find this type'
		sys.exit

def ProcessTableMapEvent(event_dict):
	data = event_dict['data'];
	#fixed data part
	table_id = ConvertStringToUnInt(data[0:6])
	tableMap.setTableMap(table_id)
	data  = data[6:len(data)]	#Table ID
	print 'table data=%d'%(ConvertStringToUnInt(data[0:2]))
	data = data[2:len(data)]	#reserved
	

	#variable data part
	#db info
	dbname_length = ConvertStringToUnInt(data[0:1]) 			#database name length
	data = data[1:len(data)]	
	db_name = data[0:dbname_length]		#database name		
	data = data[dbname_length+1:len(data)]			#+1 means '\0'
	tableMap.setDBName(table_id,db_name)

	#table info
	tablename_length = ConvertStringToUnInt(data[0:1]) 
	data = data[1:len(data)]	
	table_name = data[0:tablename_length]
	data = data[tablename_length+1:len(data)]
	tableMap.setTableName(table_id,table_name)

	#columns info
	data,columns_number = DecodePackedInt(data)	#columns number 
	tableMap.setColumnsNumber(table_id,columns_number)

	columns_type_buf = data[0:columns_number]	#column type
	tableMap.setColumnsTypeBuf(table_id,columns_type_buf)


	data = data[columns_number:len(data)]
	data,metadata_length =  DecodePackedInt(data)		#metadata  length	
	metadata = data[0:metadata_length]		#meta data
	tableMap.setMetaDataBuf(table_id,metadata)

	data = data[metadata_length:len(data)]
	column_null_bits_length = (columns_number + 7) / 8	#column bit length
	tableMap.setColumnsNullBuf(table_id,data[0:len(data)]) 
	

	#Convert Meta data and Column type
	tableMap.processMeta_ColumnType(table_id)
	

#	tableMap.printInfo(table_id)

def DecodeOneRow(buffer,table_id):
	columns_number = tableMap.getColumnsNumber(table_id)
	bitmap_size = (columns_number+7)/8    
	bitdata = buffer[0:bitmap_size]
	buffer  = buffer[bitmap_size:len(buffer)] 
	
	item_values = {}
	for i in range(columns_number):
		bits = ConvertStringToUnInt(bitdata[i/8])
		is_null = (bits >> (i%8)) & 0x01 		

		nullable = tableMap.columnNullAble(table_id,i)

		datatype = tableMap.getColumnType(table_id,i) 
		if len(tableMap.getMetaDataBuf(table_id)) == 0:
                                meta_i = 0
		else:
                                meta_i = tableMap.getMeta(table_id,i) 

		if is_null == 0:
			buffer,value,datatype = DecodeRowImageItem(buffer,datatype,meta_i)
			item_values[i] = {'value':value,'nullable':nullable,'is_null':is_null,'datatype':datatype}			
		else:
			item_values[i] = {'value':0,'nullable':nullable,'is_null':is_null,'datatype':0}
		
		i = i + 1

	return buffer,item_values

def ProcessRowEvent(event_dict):
	data = event_dict['data'];
	
	if event_dict['type_code'] == mytypes.TABLE_MAP_EVENT:
		ProcessTableMapEvent(event_dict)
		return 	
	elif event_dict['type_code'] == mytypes.WRITE_ROWS_EVENT:	
		table_id = ConvertStringToUnInt(data[0:6]) #table_id
		data = data[6:len(data)]
		reserved = data[0:2]			#reserved	
		data = data[2:len(data)]
		
		data,columns_number = DecodePackedInt(data) #columns number 

		bitmap_size = (columns_number+7)/8	    	#bitmap size 
		bitmap_data = data[0:bitmap_size]		#bitmap data
		data = data[bitmap_size:len(data)]
		row_image = data

		###
  		### decode image data
		###
		index = 0
		rows_data = {}
		while len(row_image) != 0:
			#decode row image data
			row_image,rows_data[index] = DecodeOneRow(row_image,table_id)
			index = index + 1

		FormatOutPut(mytypes.WRITE_ROWS_EVENT,table_id,rows_data)		
		OutPutJson(mytypes.WRITE_ROWS_EVENT,table_id,rows_data)		

		return 

	elif event_dict['type_code'] == mytypes.DELETE_ROWS_EVENT:
		table_id = ConvertStringToUnInt(data[0:6]) #table_id
                data = data[6:len(data)]
                reserved = data[0:2]                    #reserved       
		print 'delete data=%d'%(ConvertStringToUnInt(reserved))
                data = data[2:len(data)]

                data,columns_number = DecodePackedInt(data) #columns number 

                bitmap_size = (columns_number+7)/8              #bitmap size 
                bitmap_data = data[0:bitmap_size]               #bitmap data
                data = data[bitmap_size:len(data)]
                row_image = data

                ###
                ### decode row image data
                ###
		index = 0
		rows_data = {}
                while len(row_image) != 0:
			row_image,rows_data[index] = DecodeOneRow(row_image,table_id)
			index = index + 1
		
		FormatOutPut(mytypes.DELETE_ROWS_EVENT,table_id,rows_data)		
		OutPutJson(mytypes.DELETE_ROWS_EVENT,table_id,rows_data)		
		
	elif event_dict['type_code'] == mytypes.UPDATE_ROWS_EVENT:
		table_id = ConvertStringToUnInt(data[0:6]) #table_id
                data = data[6:len(data)]
                reserved = data[0:2]                    #reserved       
                data = data[2:len(data)]

                data,columns_number = DecodePackedInt(data) #columns number 

                bitmap_size = (columns_number+7)/8              #bitmap size 
                bitmap_data = data[0:bitmap_size]               #bitmap data
                data = data[bitmap_size:len(data)]
		row_image = data[bitmap_size:len(data)]		#only for update event

                ###
                ### decode row image data
                ###
		index = 0
		rows_data = {}
                while len(row_image) != 0:
			rows_data[index] = {}
			row_image,rows_data[index]['new'] = DecodeOneRow(row_image,table_id)	
			row_image,rows_data[index]['old'] = DecodeOneRow(row_image,table_id)
			index = index + 1
		
		FormatOutPut(mytypes.UPDATE_ROWS_EVENT,table_id,rows_data)
		OutPutJson(mytypes.UPDATE_ROWS_EVENT,table_id,rows_data)

		return  
	elif mytypes.ROTATE_EVENT == event_dict['type_code']:
		startPos = ConvertStringToUnInt(data[0:8])
		nextFileName =  data[8:len(data)]
		FormatOutPut(mytypes.ROTATE_EVENT,-1,nextFileName)
		OutPutJson(mytypes.ROTATE_EVENT,-1,nextFileName)

	else:
		return  


def OutPutJson(event_type,table_id,rows_data):
	if event_type == mytypes.ROTATE_EVENT:
		myjson = {'action':'ROTATE','row':rows_data}
		print json.dumps(myjson)
		return

	dbName = tableMap.getDBName(table_id)
        tableName = tableMap.getTableName(table_id)
	
	for i,row_item in rows_data.items():
		if event_type == mytypes.WRITE_ROWS_EVENT:
			myjson = {'dbname':dbName,'tablename':tableName,'action':'INSERT','row':row_item}
		elif event_type == mytypes.DELETE_ROWS_EVENT:
			myjson = {'dbname':dbName,'tablename':tableName,'action':'INSERT','row':row_item}
		elif event_type == mytypes.UPDATE_ROWS_EVENT:
			old_data = row_item['old']
			new_data = row_item['new']
			myjson = {'dbname':dbName,'tablename':tableName,'action':'UPDATE'}
			myjson['row'] = {'old':old_data,'new':new_data}	

	print json.dumps(myjson)
	

def FormatOutPut(event_type,table_id,rows_data):
	if mytypes.ROTATE_EVENT == event_type:
		print 'ROTATE TO %s'%rows_data
		return 
	


	dbName = tableMap.getDBName(table_id)
	tableName = tableMap.getTableName(table_id)
	
	for i,tmp_item in rows_data.items():
		if event_type == mytypes.WRITE_ROWS_EVENT:
			row_item = tmp_item
			print 'INSERT INTO %s.%s'%(dbName,tableName)
			print 'SET'
		elif event_type == mytypes.DELETE_ROWS_EVENT:
			row_item = tmp_item
			print 'DELETE FROM %s.%s'%(dbName,tableName)
                	print 'WHERE'
		elif event_type == mytypes.UPDATE_ROWS_EVENT:
			row_item = tmp_item['new']
			print 'UPDATE %s.%s'%(dbName,tableName)
			print 'WHERE'


		print ' @ROW=%d'%i
		for j,column_item in row_item.items():
			print '   @%d='%j,
			print column_item['value'],
			print ' %s  nullable=%d  is_null=%d'%\
			(column_item['datatype'],column_item['nullable'],column_item['is_null'])

		if event_type == mytypes.UPDATE_ROWS_EVENT:
			print 'SET'
			row_item = tmp_item['old']
                	for j,column_item in row_item.items():
                        	print '   @%d='%j,
                        	print column_item['value'],
                        	print ' %s  nullable=%d  is_null=%d'%\
                        	(column_item['datatype'],column_item['nullable'],column_item['is_null'])

class	MyRowLogParser:
	def __init__(self):
		self.__pos = 0
		self.__curPos = 0
		self.__buff = ''


	def GetTableMap(self):
		while True:
			myRet = self.GetEvent()
			if myRet[0] == 0:
				return 0
			if myRet[1]['type_code'] == 19:
				self.__tableMap = myRet[1]	
				return 1

	def GetEvent(self):
		if len(self.__buff[self.__curPos:]) < 19:
			return [0]
		
		event_header_tuple = struct.unpack("=ibiiih",self.__buff[self.__curPos:self.__curPos + 19])
        	event_dict = {}
        	event_dict['timestamp'] = event_header_tuple[0];
        	event_dict['type_code'] = event_header_tuple[1];
        	event_dict['server_id'] = event_header_tuple[2];
        	event_dict['event_length'] = event_header_tuple[3];
        	event_dict['next_position'] = event_header_tuple[4];
        	event_dict['flags'] = event_header_tuple[5];
		
		if len(self.__buff[self.__curPos:]) < event_dict['event_length']:
			return [0]
		event_dict['data'] = self.__buff[self.__curPos + 19:self.__curPos + event_dict['event_length']]
		self.__curPos = self.__curPos + event_dict['event_length'] 
		
#		print 'GetEvent curPos=%d'%(self.__curPos)

		return [1,event_dict]


	def GetEndRow(self):
		i = 0
		self.__events = {}
		while True:
			myRet = self.GetEvent()
                        if myRet[0] == 0:
                                return 0
			if myRet[1]['type_code'] == mytypes.WRITE_ROWS_EVENT \
				or myRet[1]['type_code'] == mytypes.DELETE_ROWS_EVENT \
				or myRet[1]['type_code'] == mytypes.UPDATE_ROWS_EVENT:

				self.__events[i] = myRet[1]
				
#				print ConvertStringToUnInt(self.__events[i]['data'][:6])
				endFlag = ConvertStringToUnInt(self.__events[i]['data'][6:8])
				i = i + 1
				if endFlag == 1:
					return 1

	def Process(self,buff):
		self.__curPos = self.__pos
		self.__buff = self.__buff + buff
		if self.GetTableMap() == 0:
			return 0
		
		if self.GetEndRow() == 0:
			return 0
	

		self.__pos = self.__curPos

		ProcessRowEvent(self.__tableMap)
		count = len(self.__events)
		for i in range(count):
			ProcessRowEvent(self.__events[i])


if __name__ == '__main__':
	filepath = '/usr/local/mysql/data/mysql-bin.000001'

        file_object = open(filepath,'rb')
	magic_number = file_object.read(4)
	buff = file_object.read(20)

	myParser = MyRowLogParser() 

	while len(buff) :
		myParser.Process(buff)	
	
		buff = file_object.read(20)
		
	file_object.close()
	
	sys.exit(0)

