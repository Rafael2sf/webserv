#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('fname')
last_name  = form.getvalue('lname')

content = "<html>\r\n<head>\r\n<title>Hello - Second CGI Program</title>\r\n</head>\r\n<body>\r\n"
if first_name != None and last_name != None:
	content += "<h2>Hello " + first_name + " " + last_name + "</h2>\r\n"
else:
	content += "<h2>Hello, anonymous person! </h2>\r\n"
content += "</body>\r\n</html>\r\n\r"

str = "HTTP/1.1 200 OK\r\n" + "Content-Length: " + str(len(content) + 1) + "\r\n" + "Content-type:text/html\r\n\r\n"
str += content

print(str)