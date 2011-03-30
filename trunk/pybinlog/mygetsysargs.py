import getopt
import sys


def GetSysArgs():
	try:
		opts,args = getopt.getopt(sys.argv[1:],"h:u:p:o:j:")
	except getopt.GetoptError:
		sys.exit(2)

	myargs = {}
	myargs['binlogname'] = args[0]
	for key,value in opts:
		key = key[1:]
		myargs[key] = value


	return myargs	


print GetSysArgs()
