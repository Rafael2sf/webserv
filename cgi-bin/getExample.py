#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, os, sys

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

now = datetime.now()
stamp = mktime(now.timetuple())

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
try:
	first_name = form.getvalue('fname')
	last_name  = form.getvalue('lname')

	if first_name != None and last_name != None:
		content = "<html>\r\n<head>\r\n<title>Hello User!</title>\r\n</head>\r\n<body>\r\n"
		content += "<h2>Hello " + first_name + " " + last_name + "</h2>\r\n</body>\r\n</html>"
		s = "HTTP/1.1 200 OK\r\ncontent-length: "
		s += str(len(content)) + "\r\n" + "content-type:text/html\r\nconnection: " + os.environ['HTTP_CONNECTION'] + "\r\n"
		s += "date: " + str(format_date_time(stamp)) + "\r\n\r\n"
		s += content

		print(s)
	else:
		sys.exit(4)
except KeyError:
	sys.exit(4)				#If fname or lname does not exist in form, the form was not well written and client receives 400
