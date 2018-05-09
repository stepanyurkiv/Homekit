#!/usr/bin/env python

import re
import sys
import os
import io
import argparse
from flask import Flask, url_for, Response, jsonify, send_file
import hashlib

FILE = "Homekit.bin"

app = Flask(__name__)


def readBinary():
	try:
			firmware_file = open(FILE, "rb")
	except Exception as err:
			print("Error: {0}".format(err.strerror))
			sys.exit(2)

	firmware_binary = firmware_file.read()
	firmware_file.close()

	return firmware_binary


def getMetadataFromBin():
	regex_homekit = re.compile(b"\x25\x48\x4f\x4d\x45\x4b\x49\x54\x5f\x45\x53\x50\x33\x32\x5f\x46\x57\x25")
	regex_name = re.compile(b"\xbf\x84\xe4\x13\x54(.+)\x93\x44\x6b\xa7\x75")
	regex_version = re.compile(b"\x6a\x3f\x3e\x0e\xe1(.+)\xb0\x30\x48\xd4\x1a")
	regex_brand = re.compile(b"\xfb\x2a\xf5\x68\xc0(.+)\x6e\x2f\x0f\xeb\x2d")

	try:
			firmware_file = open(FILE, "rb")
	except Exception as err:
			print("Error: {0}".format(err.strerror))
			sys.exit(2)

	firmware_binary = firmware_file.read()
	firmware_file.close()

	regex_name_result = regex_name.search(firmware_binary)
	regex_version_result = regex_version.search(firmware_binary)

	if not regex_homekit.search(firmware_binary) or not regex_name_result or not regex_version_result:
			print("Not a valid Homekit firmware")
			sys.exit(3)


	regex_brand_result = regex_brand.search(firmware_binary)

	name = regex_name_result.group(1).decode()
	version = regex_version_result.group(1).decode()
	brand = regex_brand_result.group(1).decode() if regex_brand_result else "unset (default is DEFAULT)"

	md5_checksum = getMD5(firmware_binary)

	data = {}
	data["name"] = name
	data["version"] = version
	data["brand"] = brand
	data["md5"] = md5_checksum

	return data


def getSize(filename):
	st = os.stat(filename)
	return st.st_size

def getMD5(firmware_binary):
	return hashlib.md5(firmware_binary).hexdigest()


app = Flask(__name__)

@app.route('/')
def api_root():
    return 'Welcome'

@app.route('/update')
def api_update():
	data = getMetadataFromBin()

	firmware_data = readBinary()	

	response = jsonify(data)
	response.mimetype = "application/json"
	response.status_code = 200

	return response

@app.route('/update/firmware')
def api_firmware():
	
	data = readBinary()
	size = str(os.path.getsize(FILE))
	#print size	

	response = Response(data)
	#response.headers.add('Content-Length', size)
	response.mimetype = "application/octet-stream"
	response.status_code = 200

	return response;
	#return app.send_static_file("Homekit.bin")	
	#return send_file(io.BytesIO("Homekit.bin"), as_attachment=False, mimetype='application/octet-stream')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Arduino Homekit Update Server')
	parser.add_argument('-f','--firmware', help='Location of the firmware file', required=False, default="Homekit.bin")
	parser.add_argument('-d','--debug', help='Debug Mode', required=False, default=False)
	args = vars(parser.parse_args())

	FILE = args["firmware"]

	print("Using firmware location: {}").format(FILE)

	app.run(host='0.0.0.0', debug=args["debug"])
	#app.run(host='0.0.0.0', ssl_context=('server.crt', 'server.key'))