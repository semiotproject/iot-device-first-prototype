#ifndef   CONNECTIONS_H
#define   CONNECTIONS_H

// Arduino<->ESP8266 Baudrate
#define ESP8266_BAUDRATE 9600
// FIXME?
// note: there is also a Serial object pointer definition
// in semiot-device.ino (Serial3)

// Debug Serial Baudrate
#define SERIAL_BAUDRATE 9600

// DHT Sensor
// DHT11 11
// DHT22 22
// DHT21 21
// AM2301 21
#define DHT_CONNECTED //TODO:
#define DHTTYPE DHT11   
#define DHT_DATA_PIN 2

// LED
#define LED_PIN 13
 
#endif // CONNECTIONS_H