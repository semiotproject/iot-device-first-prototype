# iot-device-first-prototype
Arduino MEGA 2560 and ESP8266 WiFi-Module based IoT Device prototype

## HARDWARE:
+ Arduino MEGA 2560
+ ESP8266 Device connected to Mega 2560 via Serial 3
+ DHT11 sensor connected to 2nd digital pin

![ScreenShot](https://dl.dropboxusercontent.com/u/39622126/Docs/semiot-shots/semiot-device_schem.png)
![ScreenShot](https://dl.dropboxusercontent.com/u/39622126/Docs/semiot-shots/semiot-device_bb.png)

## SOFTWARE:
+ Arduino sketches, info about used libraries inside
+ For testing I recommend to use Wireshark and libcoap 
coap-client, smcp-server and Copper Firefox Plugin:
```
coap-client -v 1 -p 5683 -m get coap://DEVICE_IP/.well-known/core
```

## ESP8266

It's highly recommended to use esp8266 firmware higher than 0.9.4 with AT commands send higher than v0.2 to get udp working more or less well.

You could find binary firmware for the esp8266 512KB flash memory version in the /software/esp8266 project repo with the PDF document containing apropriate AT commands description from Espressif Systems IOT Team (v0.23).

You could easly flash the esp8266 from lunux with the python esp8266tool like that:

```
esptool.py --port /dev/ttyACM0 --baud 115200 write_flash 0x000000 at023sdk101flash512k.bin
```

Note that after the flashing the firmware default baudrate will be 115200

If you have some problems with starting the flashing you could try to reset esp8266 VCC pin to enter the flash writing mode.

If you don't have the USB TTL converter to connect the esp8266 directly to your machine and have the arduino board, try to use the built-in arduino usb-ttl converter similar to the scheme:

![ScreenShot](http://esp8266.ru/wp-content/uploads/esp8266-arduino_bb.jpg)

We are working on the libraries we using as well to provide better results.