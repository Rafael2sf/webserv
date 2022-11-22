#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('fname')
last_name  = form.getvalue('lname')

length = 114
if first_name != None or last_name != None:
	length += len(first_name) + len(last_name)

str = "HTTP/1.1 200 OK\r\n" + "Content-Length: " + str(length) + "\r\n" + "Content-type:text/html\r\n\r\n"
str += "<html>\r\n<head>\r\n<title>Hello - Second CGI Program</title>\r\n</head>\r\n<body>\r\n"
if first_name != None or last_name != None:
	str += "<h2>Hello " + first_name + " " + last_name + "</h2>\r\n"
else:
	str += "<h2>Hello  </h2>\r\n"
str += "</body>\r\n</html>\r\n\r"

print(str)