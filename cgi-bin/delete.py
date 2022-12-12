#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb, os

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
filedir = form.getvalue('file')
if filedir != 0:
	filedir = os.path.basename(filedir)
	try:
		os.remove('/nfs/homes/daalmeid/Desktop/webserv/uploads/' + filedir)

	except:
		filedir = 'File ' + filedir + ' not found!'

	length = 108
	if filedir != None:
		length += len(filedir)

	str = "HTTP/1.1 200 OK\r\n" + "Content-Length: " + str(length) + "\r\n" + "Content-type:text/html\r\n\r\n"
	str += "<html>\r\n<head>\r\n<title>Hello - Second CGI Program</title>\r\n</head>\r\n<body>\r\n"
	if filedir != None:
		str += "<h2>" + filedir + " " + "</h2>\r\n"
	else:
		str += "<h2>Hello  </h2>\r\n"
	str += "</body>\r\n</html>\r\n\r"

	print(str)
else:
	print (response = "HTTP/1.1 404 Not Found\r\n\r")