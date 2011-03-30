#!/usr/bin/python
import struct
from myconvert import ConvertDecimalToNumber
from myconvert import ConvertStringToUnInt
from myconvert import ConvertStringToInt


dig2bytes = [0, 1, 1, 2, 2, 3, 3, 4, 4, 4]	
powers10=[1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000]
DIG_PER_DEC1 = 9

DECIMAL_BUFF_LENGTH  = 9


E_DEC_OK           =     0
E_DEC_TRUNCATED    =     1
E_DEC_OVERFLOW     =     2
E_DEC_DIV_ZERO     =     4
E_DEC_BAD_NUM      =     8
E_DEC_OOM          =    16

E_DEC_ERROR       =  31
E_DEC_FATAL_ERROR =  30

DIG_BASE  =   1000000000
DIG_MAX   =   (DIG_BASE-1)


def my_decimal_get_binary_size(precision, scale):
	return decimal_bin_size(precision, scale) 

def decimal_bin_size(precision,scale):
	intg = precision - scale 
        intg0 = intg/DIG_PER_DEC1
        frac0 = scale/DIG_PER_DEC1
        intg0x = intg-intg0*DIG_PER_DEC1
        frac0x = scale-frac0*DIG_PER_DEC1;
  
        bin_size = intg0*4 + frac0*4 + dig2bytes[intg0x]+ dig2bytes[frac0x];
  
        return bin_size

def binary2my_decimal(buf,precision, scale):
	return bin2decimal(buf,precision, scale)

#	mask = E_DEC_FATAL_ERROR
#	return check_result(int(mask), bin2decimal(buf,precision, scale))

def bin2decimal(myfrom,precision,scale):
	intg=int(precision-scale)
	intg0=intg/DIG_PER_DEC1
	frac0=scale/DIG_PER_DEC1
      	intg0x=intg-intg0*DIG_PER_DEC1
	frac0x=scale-frac0*DIG_PER_DEC1
      	intg1=intg0+(intg0x>0)
	frac1=frac0+(frac0x>0);
	if (ConvertStringToUnInt(myfrom[0]) & 0x80) != 0:
		mask = 0
	else:
		mask = -1
	
  	bin_size= decimal_bin_size(precision, scale)
	result_buffer = myfrom[0:bin_size]

	flags = (ConvertStringToUnInt(result_buffer[0]) ^ 0x80)
  
 	tmp = struct.pack('=b',flags)

	myfrom = tmp + myfrom[1:len(myfrom)]

	error = 0
  	intg1,frac1,error = FIX_INTG_FRAC_ERROR(DECIMAL_BUFF_LENGTH, intg1, frac1, error);


	if error != E_DEC_OK:
		if (intg1 < intg0+(intg0x>0)):
			mytmplen = dig2bytes[intg0x] + 4*(intg0-intg1)	
			myfrom = myfrom[mytmplen:len(myfrom)]
      			frac0=frac0x=intg0x=0
      			intg0=intg1
    		else:
      			frac0x=0
      			frac0=frac1
	to = {}
	to['sign'] = (mask != 0)
	to['intg'] = intg0*DIG_PER_DEC1+intg0x
  	to['frac'] = frac0*DIG_PER_DEC1+frac0x

	if intg0x:
		i=dig2bytes[intg0x];
	
		if i == 1:
			x = ConvertStringToUnInt(myfrom[0:1])
		elif i == 2:
       			x = ConvertStringToUnInt(myfrom[0:2])
		elif i == 3:
      			x = ConvertDecimalToNumber(myfrom[0:3]) 
		elif i == 4:
      			x = ConvertStringToUnInt(myfrom[0:4])

		myfrom = myfrom[i:len(myfrom)]

		buf = {}
		buf[0] = x ^ mask
	
		if buf[0] >= powers10[intg0x+1]:
			return E_DEC_BAD_NUM

		buf_index = 0
		if buf[0]  != 0:
			buf_index = buf_index + 1
		else:
			to['intg'] = to['intg'] - intg0x



	mystart = 0
	mystop = intg0*4
	while mystart < mystop:
		
		buf[buf_index] = ConvertDecimalToNumber(myfrom[mystart:mystart+4]) ^ mask
		mystart = mystart + 4 

		if buf[buf_index] > DIG_MAX:
			return E_DEC_BAD_NUM
		 
		if buf[buf_index] != 0:
			buf_index = buf_index + 1
		else:
			to['intg'] = DIG_PER_DEC1			

	mystop = frac0*4 + mystart
	while mystart < mystop:

		buf[buf_index] = ConvertDecimalToNumber(myfrom[mystart:mystart+4]) ^ mask	
		mystart = mystart + 4		

		if buf[buf_index] > DIG_MAX:
			return E_DEC_BAD_NUM
		buf_index = buf_index + 1	
	
	if (frac0x):
		i=dig2bytes[frac0x];
		if i == 1:
			x = ConvertDecimalToNumber(myfrom[0:1])
		elif i == 2:
			x = ConvertDecimalToNumber(myfrom[0:2])
		elif i == 3:
			x = ConvertDecimalToNumber(myfrom[0:3])
		elif i == 4:
			x = ConvertDecimalToNumber(myfrom[0:4])
		buf[buf_index] = (x ^ mask) * powers10[DIG_PER_DEC1 - frac0x];
	

		if buf[buf_index] > DIG_MAX :
			return E_DEC_BAD_NUM 
		buf_index = buf_index + 1	
		to['buf' ] = buf

	return	to	

def FIX_INTG_FRAC_ERROR(len, intg1, frac1, error):
	if intg1+frac1 > (len):
		if intg1 > (len):	
			intg1=(len)
			frac1=0
			error=E_DEC_OVERFLOW
		else:
			frac1=(len)-intg1
			error=E_DEC_TRUNCATED
	else:
		error=E_DEC_OK

	return intg1,frac1,error


def check_result(mask,result):
	if result & mask:
    		decimal_operation_results(result)
  	return result


def decimal_operation_results(result):
	return result	
