#!/usr/bin/python

import cgi, os
import cgitb; cgitb.enable()

form = cgi.FieldStorage()

fileitem = form['fileTest']

# Test if the file was uploaded
if fileitem.filename:
   # strip leading path from file name to avoid 
   # directory traversal attacks
   fn = os.path.basename(fileitem.filename)
   open('/home/rafernan/Projects/webserv/posts/' + fn, 'wb').write(fileitem.file.read())

   message = 'The file "' + fn + '" was uploaded successfully'
   
else:
   message = 'No file was uploaded'

s = "HTTP/1.1 200 OK\r\n" + "Content-Length: " + str(40 + len(message)) + "\r\n" + "Content-type:text/html\r\n\r\n"
s += "<html>\n<body>\n<p>" + message + "</p>\n</body>\n</html>\r\n\r"
print (s)