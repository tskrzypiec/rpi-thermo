

USB-UART RS232 PL2303HX Converter installation on MacOS
Install USB-UART Mac driver
http://www.prolific.com.tw/UserFiles/files/PL2303_MacOSX_1_6_1_20171018.zip
Reboot system

USB-UART RS232 PL2303HX Converter connection to RPI
configuration:
RPI UART_TXD(GPIO 14) to PL2303HX UART_RXD
RPI UART_RXD(GPIO 15) to PL2303HX UART_TXD

Boot RPI

Check Applications-Utilities folder. Click USB hardware inside
System Profiler and look for “USB-Serial Controller” or “USB-Serial Controller D”. This means that the hardware device is detected by your Mac computer.


Check if the USB serial port device is setup properly
$ ls /dev/tty.usbserial*

You should see the
tty.usbserial

connect to serial port using SCREEN program

$ screen tty.usbserial 115200

Wi-fi configuration RPI

Scan for WiFi networks(check if you see your network)
$ sudo iwlist wlan0 scan

Add network details to the Raspberry Pi

$ sudo nano /etc/wpa_supplicant/wpa_supplicant.conf

Go to the bottom of the file and add the following:

network={
    ssid="name_of_your_network"
    psk="networkPassword"
}


To verify whether it has successfully connected:

$ ifconfig wlan0

If the inet addr field has an address beside it, the Raspberry Pi has connected to the network. If not, check your password and ESSID are correct.

Run to source modified configuration file

 $ wpa_cli -i wlan0

 Status should be OK

 If not check the spellings
