#!/usr/bin/python

import cgi, os
import cgitb; cgitb.enable()

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

now = datetime.now()
stamp = mktime(now.timetuple())

form = cgi.FieldStorage()

fileitem = form['fileTest']

# Test if the file was uploaded
if fileitem.filename:
   # strip leading path from file name to avoid 
   # directory traversal attacks
   fn = os.path.basename(fileitem.filename)
   open('/nfs/homes/daalmeid/Desktop/webserv/uploads/' + fn, 'wb').write(fileitem.file.read())

   message = 'The file "' + fn + '" was uploaded successfully'
   
else:
   message = 'No file was uploaded'

s = "HTTP/1.1 200 OK\r\n" + "content-Length: " + str(40 + len(message)) + "\r\n" + "content-type: text/html\r\nconnection: " + os.environ['HTTP_CONNECTION'] + "\r\n"
s += "date: " + str(format_date_time(stamp)) + "\r\n\r\n"
s += "<html>\n<body>\n<p>" + message + "</p>\n</body>\n</html>\r\n\r"
print (s)