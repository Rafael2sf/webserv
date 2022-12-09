#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, os

# Create instance of FieldStorage 
form = cgi.FieldStorage()

fileList = os.listdir('posts/')
fileString = ""

if len(fileList) == 0:
	fileString = "404 - File not found"
	response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r"
	print (response)
else:
	for file in fileList:
		fileString += file
		fileString += '\n'
	final = fileString[:-1]
	response = "HTTP/1.1 200 OK\r\nContent-Length: " + str(len(final) + 1) + "\r\nContent-type:text/plain\r\n\r\n"
	response += final
	print (response)