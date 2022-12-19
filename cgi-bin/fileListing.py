#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, os

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
		fileString = "404 - File not found"
		response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n"
	else:
		for file in fileList:
			fileString += file
			fileString += '\n'
		fileString = fileString[:-1]
		response = "HTTP/1.1 200 OK\r\nContent-Length: " + str(len(fileString) + 1) + "\r\nContent-type:text/plain\r\n"
except:
	fileString = "404 - File not found"
	response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n"	
response += "date: " + str(format_date_time(stamp)) + "\r\n\r\n"
response += fileString
print (response)