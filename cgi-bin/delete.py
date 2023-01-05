#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, os, sys

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

def checkAccept():
	reqAccept = os.environ['HTTP_ACCEPT']
	if reqAccept.find('text/html') != -1 or reqAccept.find('text/*') != -1 or reqAccept.find('*/*') != -1:
		return 0
	return -1

now = datetime.now()
stamp = mktime(now.timetuple())

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
filedir = form.getvalue('file', 'error')
if filedir != 'error':
	filedir = os.path.basename(filedir)
	try:
		os.remove(os.getenv('DOCUMENT_ROOT') + filedir)
		s = "HTTP/1.1 200 OK\r\n"
		if os.getenv('FORCE_CODE') != '0':
			s = "HTTP/1.1 " + os.getenv('FORCE_CODE') + "\r\n"
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

	s += "connection: " + os.environ['HTTP_CONNECTION'] + "\r\n" + "date: " + str(format_date_time(stamp)) + "\r\n"
	s += "server: " + os.environ['SERVER_SOFTWARE'] + "\r\n"
	if checkAccept() != -1:
		s += "content-length: " + str(len(body)) + "\r\n" + "content-type:text/html\r\n\r\n"
		s += body

	print(s)
else:
	sys.exit(2)			#404 File not found