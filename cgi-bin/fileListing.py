#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, os, sys, errno

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

now = datetime.now()
stamp = mktime(now.timetuple())

# Create instance of FieldStorage 
form = cgi.FieldStorage()
try:
	fileList = os.listdir(os.getenv('DOCUMENT_ROOT'))
	fileString = ""

	if len(fileList) == 0:
		sys.exit(2)
	else:
		for file in fileList:
			if os.path.isfile(os.getenv('DOCUMENT_ROOT') + file):
				fileString += file
				fileString += '\n'
		if fileString == "":
			raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT))
		fileString = fileString[:-1]
		response = "HTTP/1.1 200 OK\r\nContent-Length: " + str(len(fileString) + 1) + "\r\nContent-type:text/plain\r\nconnection: " + os.environ['HTTP_CONNECTION'] + "\r\n"
except IOError as e:
		if e.errno == 2:
			sys.exit(2)
		elif e.errno == 13:
			sys.exit(3)
		elif e.errno == 21:
			sys.exit(3)
response += "date: " + str(format_date_time(stamp)) + "\r\n\r\n"
response += fileString
print (response)