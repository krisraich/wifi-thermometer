# 
# BMP to H-Icon for wifi-thermometer
# Author: robsl2314 (robert@kathreins.at)
# 
# This script reads a bmp file and generates a H-Image String for direct usage
#
import os.path, sys
from PIL import Image

filename = sys.argv[1]

if os.path.isfile(filename):
	## Method with PIL
	im = Image.open(filename, "r")
	bitCounter = 0
	outByte = 0
	output = "const unsigned char " + filename[:-4] + "[" + str(int(len(im.getdata()) / 8)) + "] PROGMEM = " + "{"
	for px in im.getdata():
		if px > 0:
			outByte = outByte + 1
		bitCounter = bitCounter + 1
		if(bitCounter >= 8):
			#output = output + hex(outByte) + ", " # e.g. 0xf
			output = output + '0x%0*X' % (2,outByte) + ", " # e.g. 0x0F
			outByte = 0
			bitCounter = 0
		else:
			outByte = outByte << 1
	output = output.strip(", ") + "}"
	print(output)
	
	## Own implementation - Not jet finished
	#with open(filename, 'rb') as f:
	#	data = bytearray(f.read())
	#	offset = ((data[12] << 24) | (data[11] << 16 ) | (data[10] << 8) | data[9])
	#	print(str(data[9::11]))
	#	print(offset)
	#	output = "const unsigned char " + filename + "[" + str(len(data)) + "] = " + "{"
	#	for x in data:
	#		output = output + hex(x) + ","

	#output = output.strip(",") + "}"
	#print(output)
else:
	print("Not a File")