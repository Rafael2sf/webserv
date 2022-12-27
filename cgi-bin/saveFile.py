#!/usr/bin/python

import cgi, os, sys, time

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

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
	
	else:
		message = 'No file was uploaded'
		message =  "<html>\n<body>\n<p>" + message + "</p>\n</body>\n</html>\r\n\r"

	s = "HTTP/1.1 201 Created\r\n" + "content-Length: " + str(len(message)) + "\r\n" + "content-type: text/html\r\nconnection: " + os.environ['HTTP_CONNECTION'] + "\r\n"
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