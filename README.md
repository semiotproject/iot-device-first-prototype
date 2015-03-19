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
coap-client:
```
coap-client -v 1 -p 5683 -m get coap://DEVICE_IP/.well-known/core
```
