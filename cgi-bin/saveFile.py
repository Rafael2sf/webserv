#!/usr/bin/python

import cgi, os, sys, time

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
form = cgi.FieldStorage()

try:
	fileitem = form['fileTest']
	# Test if the file was uploaded
	if fileitem.filename:
		# strip leading path from file name to avoid 
		# directory traversal attacks
		fn = os.path.basename(fileitem.filename)
		open(os.getenv('DOCUMENT_ROOT') + fn, 'wb').write(fileitem.file.read())

		message = 'The file "' + fn + '" was uploaded successfully'
		s = "HTTP/1.1 201 Created\r\nlocation: " + fn + "\r\n"

	else:
		message = 'No file was uploaded'
		message =  "<html>\n<body>\n<p>" + message + "</p>\n</body>\n</html>\r\n\r"
		s = "HTTP/1.1 200 OK\r\n"
		if os.getenv('FORCE_CODE') != '0':
			s = "HTTP/1.1 " + os.getenv('FORCE_CODE') + "\r\n"

	s += "connection: " + os.environ['HTTP_CONNECTION'] + "\r\nserver: " + os.environ['SERVER_SOFTWARE'] + "\r\n"
	s += "date: " + str(format_date_time(stamp)) + "\r\n"
	if checkAccept() != -1:
		s += "content-length: " + str(len(message)) + "\r\n" + "content-type: text/html\r\n\r\n"
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