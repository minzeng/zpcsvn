import pymysql
import struct
from util import int2byte
from util import byte2int
from util import join_bytes


COM_BINLOG_DUMP = 18

def pack_int32(n):
        return struct.pack('BBBB', n&0xFF, (n>>8)&0xFF, (n>>16)&0xFF,(n>>24)&0xFF)

def pack_int16(n):
        return struct.pack('BB', n&0xFF, (n>>8)&0xFF)

def pack_int24(n):
    return struct.pack('BBB', n&0xFF, (n>>8)&0xFF, (n>>16)&0xFF)


class MyDumpBinLog:
	"""Recv data from pymysql.connection.socket"""
	def __init__(self,conn):
		self.__conn = conn
		self.__socket = conn.socket
		
	def start_dump(self,filename,startpos):
		args = pack_int32(startpos)            #start position
        	args = args + pack_int16(0) #binlog flags
        	args = args + pack_int32(1)     #server_id of the slave
        	args = args + filename        #binlog file name (optional)
        	self.__conn._execute_command(COM_BINLOG_DUMP, args)

	def recv_packet(self):
		packet_header = self.__socket.recv(4)
		while len(packet_header) < 4:
			d = self.__socket.recv(4 - len(packet_header))
			if len(d) == 0:
				print 'len === 0'
				return ''
				raise OperationalError(2013, "Lost connection to MySQL server during query")
			packet_header += d
		packet_length_bin = packet_header[:3]
		self.__packet_number = byte2int(packet_header[3])
		# TODO: check packet_num is correct (+1 from last packet)

		bin_length = packet_length_bin + int2byte(0)  # pad little-endian number
		bytes_to_read = struct.unpack('<I', bin_length)[0]

    		payload_buff = []  # this is faster than cStringIO
    		while bytes_to_read > 0:
      			recv_data = self.__socket.recv(bytes_to_read)
      			if len(recv_data) == 0:
				print 'len == 0'
				return ''
            			raise OperationalError(2013, "Lost connection to MySQL server during query")
		
			payload_buff.append(recv_data)
			bytes_to_read -= len(recv_data)
		self.__data = join_bytes(payload_buff)

		return self.__data

