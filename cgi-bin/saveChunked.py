#!/usr/bin/python3

# Import modules for CGI handling 
import os, sys

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

now = datetime.now()
stamp = mktime(now.timetuple())
file = sys.stdin.read() 


try:
	open(os.getenv('DOCUMENT_ROOT') + "test.txt", 'wb').write(bytes(file, 'UTF-8'))
	message = 'The file "test.txt" was uploaded successfully'
	s = "HTTP/1.1 201 Created\r\nlocation: " + "test.txt" + "\r\n"
	s += "content-length: " + str(len(message)) + "\r\n" + "content-type: text/html\r\nconnection: " + os.environ['HTTP_CONNECTION'] + "\r\n"
	s += "date: " + str(format_date_time(stamp)) + "\r\n\r\n"
	s += message
	print(s)
except IOError as e:
	if e.errno == 2:
		sys.exit(2)			#404 File not found
	elif e.errno == 13:
		sys.exit(3)			#403 Forbidden
	elif e.errno == 21:
		sys.exit(3)			#403 Forbidden
except KeyError:
	sys.exit(4)				#If fileTest does not exist in form, the form was not well written and client receives 400