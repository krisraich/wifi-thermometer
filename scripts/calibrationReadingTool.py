#
# ADC Calibration Tool for wifi-thermometer
# Author: robsl2314 (robert@kathreins.at)
#
# This script connects up to your ESP via an entered COM Port
# It creates a semicolon seperated CSV with the sensor data of
# ADC0-5 with its entered values
#


import serial, csv
from time import sleep

print("Enter name for the CSV: ")
filename = input()
with open(filename + '.csv', 'w') as csvfile:
    fieldnames = ['ADC0', 'ADC1', 'ADC2', 'ADC3', 'ADC4', 'ADC5', 'Value']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames, delimiter=';', quoting=csv.QUOTE_NONE, lineterminator='\n')
    writer.writeheader()

    # opening serial interface
    print("Enter COM Port (e.g. 3): ")
    comPort = input()
    outerLoop = True
    ser = serial.Serial()
    ser.baudrate = 115200
    ser.port = 'COM' + comPort
    while outerLoop:
        try:
            print("Try connecting...")
            ser.open()

            ser.write("0".encode('utf8'))
            data = ser.readline()
            print(data)
            if data != "b'true\\r\\n'":
                print("Connected to something, but not a wifi thermo.. exiting")
                #  break

            ser.write("0".encode('utf8'))
            print("Connected... To exit loop enter 'exit'")
            print("Select Mode:")
            print("[1] Calibrate ADC")
            print("[2] Set WiFi Password")
            print("[3] Remove WiFi Password")
            print("[4] Set WiFi SSID")

            while True:
                val = input()

                if val == 'exit':
                    outerLoop = False
                    break

                ser.reset_input_buffer()
                ser.write("1".encode('utf8'))  # goto calibration

                ser.write(val.encode('utf8'))  # write something to trigger reading

                # #write mode
                # ser.write(val[0].encode('utf8'))
                # data = ser.readline()
                # if data == 'false':
                #     print("Unknown mode... exiting")
                #     outerLoop = False
                #     break



                data = ser.readline()
                data = str(data)[2:-5].strip().split()
                data.append(val)
                writer.writerow(dict(zip(fieldnames, data)))

        except:
            print("No connection: ")
            print(ValueError)
            sleep(0.5)
    print("Exiting...")
