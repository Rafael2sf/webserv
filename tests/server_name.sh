#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: ./prog port"
	exit 1
fi

curl_test() {

	curl --resolve *:$1:127.0.0.1 $2:$1/
	if [ $? -ne 0 ]; then
		exit 1
	fi
	echo ""
}

curl_test $1 "example"
curl_test $1 "www.example"
curl_test $1 "dev.example"
curl_test $1 "a.www.example"
curl_test $1 "dev.www.example"
curl_test $1 "localhost"
