#!/usr/bin/python

import requests

def gen():
	yield bytes('<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n        <meta charset=\"UTF-8\">\n        <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n        <link rel=\"icon\" type=\"image/x-icon\" href=\"/favicon.ico\">\n        <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.6.1/jquery.min.js\"></script>\n        <title>Index</title>\n</head>\n<body>\n        <h1>HELLO WOOOOOOORLD!</h1>\n        <br>\n        <form action=\"/getExample.py\" method=\"GET\">\n                <label for=\"fname\">First name:</label><br>\n                <input type=\"text\" id=\"fname\" name=\"fname\" value=\"John\"><br>\n                <label for=\"lname\">Last name:</la', 'utf-8')
	yield bytes('bel><br>\n                <input type=\"text\" id=\"lname\" name=\"lname\" value=\"Doe\"><br><br>\n                <input type=\"submit\" value=\"Submit\">\n          </form>\n        <br>\n        <h2>Upload</h2>\n        <form enctype=\"multipart/form-data\" action=\"/saveFile.py\" method=\"POST\">\n                <label for=\"fileTest\">Upload file:</label><br>\n                <input type=\"file\"\n                                id=\"fileTest\"\n                                name=\"fileTest\"><br>\n                <input type=\"submit\" value=\"Submit\">\n        </form>\n        <br>\n        <h2>Deletion</h2>\n        <select id=\"del_files\" name=\"file_select\" >\n                <option disabled>Select file to delete:</option>\n        </select>\n        <script src=\"./js/select.js\"></script>\n        <button type=\"button\" id=\"btn\">Delete</butto', 'utf-8')
	yield bytes(' ', 'utf-8')
	yield bytes('n>\n        <script src=\"./js/delete.js\"></script>\n        <p>\n                <img src=\"./images/smoking.jpg\" alt=\"python image\">\n        </p>\n\n</body>\n</html>', 'utf-8')


res = requests.post('http://localhost:8000/saveSimple.py', data=gen())
print(res)
print(res.content)