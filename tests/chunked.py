#!/usr/bin/python

import requests

def gen():
	yield bytes('hi', 'utf-8')
	yield bytes(' there rhrtjhrt', 'utf-8')
	yield bytes(' play', 'utf-8')

res = requests.post('http://localhost:8000/saveSimple.py', data=gen())

print(res)
print(res.content)
