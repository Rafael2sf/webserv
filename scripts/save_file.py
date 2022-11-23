#!/usr/bin/python

import cgi, os
import cgitb; cgitb.enable()
import sys

#env = int(os.getenv('CONTENT_LENGTH'))
#input = sys.stdin.read(730)
#print (input)
form = cgi.FieldStorage()

fileitem = form['fileTest']

# Test if the file was uploaded
if fileitem.filename:
   # strip leading path from file name to avoid 
   # directory traversal attacks
   fn = os.path.basename(fileitem.filename)
   open('/nfs/homes/daalmeid/Desktop/webserv/posts/' + fn, 'wb').write(fileitem.file.read())

   message = 'The file "' + fn + '" was uploaded successfully'
   
else:
   message = 'No file was uploaded'
   
print ("""\
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 105\n
<html>
   <body>
      <p>%s</p>
   </body>
</html>\r\n\r""" % (message,))