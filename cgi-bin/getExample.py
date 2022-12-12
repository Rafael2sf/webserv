#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb 

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

now = datetime.now()
stamp = mktime(now.timetuple())

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('fname')
last_name  = form.getvalue('lname')

if first_name != None and last_name != None:
	content = "<html>\r\n<head>\r\n<title>Hello From Our CGI Script</title>\r\n</head>\r\n<body>\r\n"
	content += "<h2>Hello " + first_name + " " + last_name + "</h2>\r\n"
	s = "HTTP/1.1 200 OK\r\n"
else:
	content = "<html>\r\n<head>\r\n<title>Error 400 - File Not Found</title>\r\n</head>\r\n<body>\r\n"
	content += "<h2>400</h2>\r\n"
	s =  "HTTP/1.1 400 Bad Request\r\n"
content += "</body>\r\n</html>\r\n\r"

s += "content-length: " + str(len(content) + 1) + "\r\n" + "content-type:text/html\r\n"
s += "date: " + str(format_date_time(stamp)) + "\r\n\r\n"
s += content

print(s)