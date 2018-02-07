# 
# ADC Calibration Tool for wifi-thermometer
# Author: robsl2314 (robert@kathreins.at)
# 
# This script connects up to your ESP via an entered COM Port
# It creates a semicolon seperated CSV with the sensor data of 
# ADC0-5 with its entered values
# 


import serial, csv


print("Enter name for the CSV: ")
filename = input()
with open(filename + '.csv', 'w') as csvfile:
	fieldnames = ['ADC0', 'ADC1', 'ADC2', 'ADC3', 'ADC4', 'ADC5', 'Value']
	writer = csv.DictWriter(csvfile, fieldnames=fieldnames, delimiter=';', quoting=csv.QUOTE_NONE, lineterminator='\n')
	writer.writeheader()
	
	# opening serial interface
	print("Enter COM Port (e.g. 3): ")
	comPort = input()
	ser = serial.Serial()
	ser.baudrate = 115200
	ser.port = 'COM' + comPort
	try:
		ser.open()
		print("To exit loop enter 'exit'")
		while True:
			print("Enter current Value: ")
			val = input()
			if (val == 'exit'):
				break
			ser.reset_input_buffer()
			ser.write("+".encode('utf8'))
			data = ser.readline()
			data = str(data)[2:-5].strip().split()
			data.append(val)
			writer.writerow(dict(zip(fieldnames, data)))
			
	except ValueError:
		print("Error: ")
		print(ValueError)