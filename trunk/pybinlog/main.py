import pymysql
from mydumpbinlog import MyDumpBinLog 
from mybinlogread import MyRowLogParser
from mygetsysargs import GetSysArgs
from myconvert import ConvertStringToUnInt 
import struct
import string





if __name__ == '__main__':
	
	#get command line args
	myargs = GetSysArgs()
	conn = pymysql.connect(host=myargs['h'], port=3306, user=myargs['u'], passwd=myargs['p'], db='mysql')


	#create connection with master
	mydump = MyDumpBinLog(conn)
	myargs['j'] = string.atoi(myargs['j']) #ConvertStringToUnInt(myargs['j'])	


	#send COM_BIN_DUMP command
	#WARING:start position need to remember by file
	mydump.start_dump(myargs['binlogname'],myargs['j'])

	#start to recevie master's dump
	data = ''

	myParser = MyRowLogParser()

	while True:
		recv_data = mydump.recv_packet()
		recv_data = recv_data[1:]
		if len(recv_data) == 0:
			break
                myParser.Process(recv_data)
