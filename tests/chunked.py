#!/usr/bin/python

import requests

def gen():
	yield bytes('hi', 'utf-8')
	yield bytes(' there rhrtjhrt', 'utf-8')
	yield bytes(' play', 'utf-8')

headers = {'content-type': 'application/x-www-form-urlencoded; charset=UTF-8'}

res = requests.post('http://localhost:8000/saveChunked.py', data=gen(), headers=headers)

print(res)