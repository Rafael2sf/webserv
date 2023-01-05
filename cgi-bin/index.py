#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, os, sys, errno

# Date header creation
from wsgiref.handlers import format_date_time
from datetime import datetime
from time import mktime

def checkAccept():
	reqAccept = os.environ['HTTP_ACCEPT']
	if reqAccept.find('text/html') != -1 or reqAccept.find('text/*') != -1 or reqAccept.find('*/*') != -1:
		return 0
	return -1

if checkAccept() == -1:
	sys.exit(5) #406 Not Acceptable

now = datetime.now()
stamp = mktime(now.timetuple())

# Create instance of FieldStorage 
form = cgi.FieldStorage()
fileString = ""
fileString ="""<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset="UTF-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<link rel="stylesheet" href="../www/css/index.py.css">
	<title>webserv</title>
</head>
<script src="https://code.jquery.com/jquery-3.5.1.min.js"></script>
<script>
	function imageDelete(f) {
		try {
			console.log(f);
			$.ajax({
				type: "DELETE",
				url: "/delete.py",
				data: {file: f},
				success: function() {
					alert("File " + f + " erased!");
					location.reload();
				},
				error: function( jqXHR, textStatus, errorThrown) {
					alert("ERROR: " + errorThrown);
				}
			});
		} 
		catch (error) {
			alert("File does not exist!");
		}
	}
</script>
<body>
	<div class="menu-form">
		<form onsubmit="location.reload()" enctype="multipart/form-data" action="/saveFile.py" method="POST">
			<input type="file" id="fileTest" name="fileTest"/>
			<input type="submit" value="+"/>
		</form>
	</div>
	<div class="menu-images">"""
try:
	fileList = os.listdir(os.getenv('DOCUMENT_ROOT'))

	if len(fileList) > 0:
		for file in fileList:
			if os.path.isfile(os.getenv('DOCUMENT_ROOT') + file):
				if file.endswith(('.mp4', '.mp3')):
					fileString += "<div class=\"image-card\">"
					fileString += "<a href=\"/uploads/" + file + "\">	</a>"
					fileString += "<button onclick=imageDelete('" + file + "')>X</button>"
					fileString += "<label>" + file + "</label>"
					fileString += "<video autoplay muted><source type=\"video/mp4\" src=\"/uploads/" + file + "\"></source></video></div>\n"
				else:
					fileString += "<div class=\"image-card\">"
					fileString += "<a href=\"/uploads/" + file + "\">	</a>"
					fileString += "<button onclick=imageDelete('" + file + "')>X</button>"
					fileString += "<label>" + file + "</label>"
					fileString += "<img src=\"/uploads/" + file + "\"/></div>\n"
		if fileString == "":
			raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT))
		fileString = fileString[:-1]
except IOError as e:
	_
fileString += """
	</div>
</body>
</html>"""
response = "HTTP/1.1 200 OK\r\n"
if os.getenv('FORCE_CODE') != '0':
	response = "HTTP/1.1 " + os.getenv('FORCE_CODE') + "\r\n"
response += "Content-Length: " + str(len(fileString) + 1) \
+ "\r\nContent-type:text/html\r\nConnection: " + os.environ['HTTP_CONNECTION'] \
+ "\r\nServer: " + os.environ['SERVER_SOFTWARE'] + "\r\n" \
+ "Date: " + str(format_date_time(stamp)) + "\r\n\r\n"
response += fileString
print (response)
