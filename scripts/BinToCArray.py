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
	with open(filename, 'rb') as f:
		data = bytearray(f.read())
		output = "const unsigned char " + filename[:-4] + "[" + str(len(data)) + "] = " + "{"
		for nextByte in data:
			#output = output + hex(nextByte) + "," # e.g. 32 to 0xf
			output = output + '0x%0*X' % (2,nextByte) + "," # e.g. 32 to 0x0F

	output = output.strip(",") + "}"
	fout = open(filename + ".BinArray.txt", "w")
	fout.write(output)
	fout.close()
	print(output)
else:
	print("Not a File")