import configparser
import serial
import time

# Get current configuration
config = configparser.ConfigParser()
config.read('config.ini')

# Check which port to use and open serial connection
s = 0
if (config['Serial'].getboolean('useDevGlove')):
	s = serial.Serial(port=config['Serial']['devPort'], baudrate=9600)
else:
	s = serial.Serial(port=config['Serial']['livePort'], baudrate=9600)

print("Ramping up...")
for i in range(0, 100, 2):
	s.write(bytes([ 255, 2, i, 20, 0 ]))
	print(i)
	time.sleep(0.2)
	
print("\nRamping down...")
for i in range(100, 0, -2):
	s.write(bytes([ 255, 2, i, 20, 0 ]))
	print(i)
	time.sleep(0.2)