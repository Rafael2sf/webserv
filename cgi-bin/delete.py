#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, os, sys

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
filedir = form.getvalue('file', 'error')
if filedir != 'error':
	filedir = os.path.basename(filedir)
	try:
		os.remove(os.getenv('DOCUMENT_ROOT') + filedir)
		s = "HTTP/1.1 200 OK\r\n"
		body = "<html>\n<head>\n<title>Delete script</title>\n</head>\n<body>\n"

	except IOError as e:
		
		if e.errno == 2:
			sys.exit(2)			#404 File not found
		elif e.errno == 13:
			sys.exit(3)			#403 Forbidden
		elif e.errno == 21:
			sys.exit(3)			#403 Forbidden

	body += "<h2>File" + filedir + "was deleted</h2>\n"
	body += "</body>\n</html>"

	s += "Content-Length: " + str(len(body)) + "\r\n" + "Content-type:text/html\r\nconnection: " + os.environ['HTTP_CONNECTION'] + "\r\n\r\n"
	s += body

	print(s)
else:
	sys.exit(2)			#404 File not found