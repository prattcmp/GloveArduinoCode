import sys
sys.dont_write_bytecode = True

import pyble
from pyble.handlers import PeripheralHandler, ProfileHandler

from PyQt5 import QtCore, QtWidgets

class MotorControl(ProfileHandler):
    UUID = "15db5d20-50d4-4370-a439-754e7182cb54"
    _AUTOLOAD = True

    names = {
        "15db5d20-50d4-4370-a439-754e7182cb54": "Motor 0",
        "15db5d21-50d4-4370-a439-754e7182cb54": "Motor 1",
        "15db5d22-50d4-4370-a439-754e7182cb54": "Motor 2",
        "15db5d23-50d4-4370-a439-754e7182cb54": "Motor 3",
        "15db5d24-50d4-4370-a439-754e7182cb54": "Motor 4",
        "15db5d25-50d4-4370-a439-754e7182cb54": "Motor 5",
        "15db5d26-50d4-4370-a439-754e7182cb54": "Motor 6"
    }

    def intialize(self):
        print("Motor Control initialized")
        pass

    def on_read(self, characteristic, data):
        ans = []
        for b in data:
            ans.append("%02X" % ord(b))
        ret = "0x" + "".join(ans)

        return ret

class GlovePeripheral(PeripheralHandler):
    def initialize(self):
        self.addProfileHandler(MotorControl)

    def on_connect(self):
        print(self.peripheral, "connected")

    def on_disconnect(self):
        print(self.peripheral, "disconnected")

    def on_rssi(self, value):
        print(self.peripheral, "update RSSI:", value)
'''
class BLEThread(QtCore.QThread):
    def __init__(self, BluetoothControl):
        QtCore.QThread.__init__(self)
        self.BluetoothControl = BluetoothControl

    def run(self):
        target = None

        while True:
            try:
                target = self.BluetoothControl.centralManager.startScan()
                if target:
                    print(target)
                    if target.name == "HapticGloveA":
                        break
            except Exception as e:
                print(e)

        target.delegate = MyPeripheral

        peripheral = self.BluetoothControl.centralManager.connectPeripheral(target)

        for service in peripheral:
            print(service)
            for characteristic in service:
                print(c, ":", c.value)

        characteristic = peripheral["HapticGloveProfile"]["M0"]
        print(characteristic.value)

        centralManager.disconnectPeripheral(peripheral)
'''
class BluetoothControl(QtWidgets.QWidget):
    def __init__(self, parent = None):
        super(BluetoothControl, self).__init__(parent)
        
        self.centralManager = pyble.CentralManager()

        target = None
        print("working...")
        while True:
            try:
                target = self.centralManager.startScan()
                if target:
                    if target.name == "HapticGloveA":
                        print("WE FOUND IT")
                        break
            except Exception as e:
                print(e)

        print("MADE IT HERE")
        target.delegate = MyPeripheral
        print("MADE IT HERE TOO")
        peripheral = self.centralManager.connectPeripheral(target)
        print("AND HERE")
        print()
        for service in peripheral:
            print(service)
            for characteristic in service:
                print(c, ":", c.value)

        characteristic = peripheral["HapticGloveProfile"]["M0"]
        print(characteristic.value)

        self.centralManager.disconnectPeripheral(peripheral)
        '''
        self.ble_thread = BLEThread(self)
        self.ble_thread.start()
        '''       

        layout = QtWidgets.QVBoxLayout()
        
        self.motorLabel = QtWidgets.QLabel("Motor:")
        self.motorSelect = QtWidgets.QComboBox()
        self.motorSelect.addItems(['0', '1', '2', '3', '4', '5', 'All'])


        self.durationLabel = QtWidgets.QLabel("Duration:")
        self.durationSelect = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.durationSelect.setMinimum(0)
        self.durationSelect.setMaximum(100)
        self.durationSelect.setValue(100)
        self.durationSelect.setTickPosition(QtWidgets.QSlider.TicksBelow)
        self.durationSelect.setTickInterval(5)


        self.intensityLabel = QtWidgets.QLabel("Intensity:")
        self.intensitySelect = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.intensitySelect.setMinimum(0)
        self.intensitySelect.setMaximum(100)
        self.intensitySelect.setValue(50)
        self.intensitySelect.setTickPosition(QtWidgets.QSlider.TicksBelow)
        self.intensitySelect.setTickInterval(5)
        
        self.sendButton = QtWidgets.QPushButton("Send")
        self.sendButton.setDefault(True)
        self.sendButton.clicked.connect(self.sendCommand)

        layout.addWidget(self.motorLabel)
        layout.addWidget(self.motorSelect)

        layout.addWidget(self.durationLabel)
        layout.addWidget(self.durationSelect)

        layout.addWidget(self.intensityLabel)
        layout.addWidget(self.intensitySelect)

        layout.addWidget(self.sendButton)
        
        self.setLayout(layout)
        self.setWindowTitle("Glove Bluetooth Control")

    def sendCommand(self):
        #FIXME
        pass

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)

    exec = BluetoothControl()

    exec.show()
    sys.exit(app.exec_())


