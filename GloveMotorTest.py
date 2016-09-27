import serial, time

# [0xFF][orientation][intensity][duration][checksum - never 255. sender will truncate to 254 rather than send a 255]
s = serial.Serial(port='/dev/cu.usbmodem457341', baudrate=9600)

# Run through each motor twice
print("Running each motor individually...")
time.sleep(1)
for i in range(0, 10):
	motor = i // 2
	s.write(bytes([ 255, motor, 90, 100, 0 ]))
	print("Motor", motor)
	time.sleep(3)


# Test all of them
print("Running every motor...")
time.sleep(3)
s.write(bytes([ 255, 5, 90, 100, 0 ]))
time.sleep(0.1)
s.write(bytes([ 255, 5, 90, 100, 0 ]))
time.sleep(0.1)
s.write(bytes([ 255, 5, 90, 100, 0 ]))
time.sleep(3)

# Test different durations
print("Running every motor at different durations...")
s.write(bytes([ 255, 5, 90, 20, 0 ]))
time.sleep(2)
s.write(bytes([ 255, 5, 90, 50, 0 ]))
time.sleep(2)
s.write(bytes([ 255, 5, 90, 75, 0 ]))
time.sleep(2)
s.write(bytes([ 255, 5, 90, 100, 0 ]))

print("Run complete.")
