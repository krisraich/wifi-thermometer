# by robsl2314
# install pyserial first

import serial

ser = serial.Serial()
ser.baudrate = 115200
ser.port = 'COM3'
ser.open()
for x in range(0,200):
  ser.readline()
