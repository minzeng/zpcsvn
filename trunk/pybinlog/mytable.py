#!/usr/bin/python
from myconvert import ConvertStringToUnInt
import mytypes

class TableMap:
	_tableMap = {}

	def setTableMap(self,tableID):
		self._tableMap[tableID] = {}

	def setDBName(self,tableID,dbName):
		self._tableMap[tableID]['db_name'] = dbName
	
	def getDBName(self,tableID):
		return self._tableMap[tableID]['db_name']

	def setTableName(self,tableID,tableName):
		self._tableMap[tableID]['table_name'] = tableName
	
	def getTableName(self,tableID):
		return self._tableMap[tableID]['table_name']

	def setColumnsNumber(self,tableID,columnsNumber):
		self._tableMap[tableID]['columns_number'] = columnsNumber

	def getColumnsNumber(self,tableID):
		return self._tableMap[tableID]['columns_number']

	def setColumnsTypeBuf(self,tableID,columnsType):
		self._tableMap[tableID]['columns_type_buf'] = columnsType 

	def getColumnsTypeBuf(self,tableID):
		return self._tableMap[tableID]['columns_type_buf']

	def setMetaDataBuf(self,tableID,metaData):
		self._tableMap[tableID]['meta_data_buf'] = metaData

	def getMetaDataBuf(self,tableID):
		return self._tableMap[tableID]['meta_data_buf']


	def setColumnsNullBuf(self,tableID,columnNullBits):
		self._tableMap[tableID]['columns_null_buf'] = columnNullBits	

	def getMeta(self,tableID,pos):
		return  self._tableMap[tableID]['meta_data'][pos]

	def getColumnType(self,tableID,pos):
		return  self._tableMap[tableID]['columns_type'][pos]

	def decodeColumnsDataType(self,tableID):
		columnsNumber = self.getColumnsNumber(tableID) 
		columnsTypeBuf =  self.getColumnsTypeBuf(tableID)
		metaDataBuf = self.getMetaDataBuf(tableID) 
		self._tableMap[tableID]['columns_type'] = {}
		for i in range(columnsNumber):
			columnType_i = ConvertStringToUnInt(columnsTypeBuf[i]) 
			meta_i = self.getMeta(tableID,i)
			if columnType_i == mytypes.MYSQL_TYPE_STRING:
				real_type = meta_i >> 8
				if real_type == mytypes.MYSQL_TYPE_ENUM or real_type == mytypes.MYSQL_TYPE_SET:
					columnType_i = real_type
				elif columnType_i == mytypes.MYSQL_TYPE_DATE:
					columnType_i = mytypes.MYSQL_TYPE_NEWDATE
			self._tableMap[tableID]['columns_type'][i] = columnType_i 
					
	def decodeMetaData(self,tableID):
		self._tableMap[tableID]['meta_data'] = {}
		columnsNumber = self.getColumnsNumber(tableID)	
		columnsTypeBuf =  self.getColumnsTypeBuf(tableID)
		metaDataBuf = self.getMetaDataBuf(tableID)

		j = 0
		for i in range(columnsNumber):
			columnType_i = ConvertStringToUnInt(columnsTypeBuf[i])
			if columnType_i in (mytypes.MYSQL_TYPE_TINY_BLOB,\
			mytypes.MYSQL_TYPE_BLOB,mytypes.MYSQL_TYPE_MEDIUM_BLOB,\
			mytypes.MYSQL_TYPE_LONG_BLOB,mytypes.MYSQL_TYPE_DOUBLE,\
			mytypes.MYSQL_TYPE_FLOAT,mytypes.MYSQL_TYPE_GEOMETRY):
				meta_j = ConvertStringToUnInt(metaDataBuf[j])
				j = j + 1
				self._tableMap[tableID]['meta_data'][i] = meta_j				

			elif columnType_i in (mytypes.MYSQL_TYPE_SET,\
			mytypes.MYSQL_TYPE_ENUM,mytypes.MYSQL_TYPE_STRING):
				meta_j = ConvertStringToUnInt(metaDataBuf[j])
				j = j + 1
				x = meta_j << 8
				meta_j = ConvertStringToUnInt(metaDataBuf[j])
				j = j + 1
				self._tableMap[tableID]['meta_data'][i] = x + meta_j 

			elif columnType_i == mytypes.MYSQL_TYPE_BIT:
				meta_j = ConvertStringToUnInt(metaDataBuf[j])
				j = j + 1
				x = meta_j
				meta_j = ConvertStringToUnInt(metaDataBuf[j])
				j = j + 1
				x = x + meta_j << 8 
				self._tableMap[tableID]['meta_data'][i] = x

			elif columnType_i == mytypes.MYSQL_TYPE_VARCHAR: #These types store two bytes.
				meta_j =  ConvertStringToUnInt(metaDataBuf[j:j+2])			
				j = j + 2
				self._tableMap[tableID]['meta_data'][i] = meta_j

			elif columnType_i == mytypes.MYSQL_TYPE_NEWDECIMAL:
				meta_j = ConvertStringToUnInt(metaDataBuf[j])
				j = j + 1
				x = meta_j << 8
				meta_j = ConvertStringToUnInt(metaDataBuf[j])
				j = j + 1
				x = x + meta_j
				self._tableMap[tableID]['meta_data'][i] = x	
			else:
		  		self._tableMap[tableID]['meta_data'][i] = 0	
		
	def decodeColumnsBitNull(self,tableID):
		columnsNumber = self.getColumnsNumber(tableID) 	
		columns_null_buf = self._tableMap[tableID]['columns_null_buf']
		self._tableMap[tableID]['column_null_bits'] = {}
		for i in range(columnsNumber):
			bits = ConvertStringToUnInt(columns_null_buf[i/8])
			self._tableMap[tableID]['column_null_bits'][i] = \
			((bits & (1 << (i%8))) == (1 << (i%8)))

	def processMeta_ColumnType(self,tableID):
		self.decodeMetaData(tableID)	
		self.decodeColumnsDataType(tableID)
		self.decodeColumnsBitNull(tableID)

	def columnNullAble(self,tableID,pos):
		return self._tableMap[tableID]['column_null_bits'][pos]

	def printInfo(self,tableID):
		print self._tableMap[tableID]
