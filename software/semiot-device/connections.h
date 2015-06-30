#ifndef   CONNECTIONS_H
#define   CONNECTIONS_H

#define COAP_PORT 5683

// Arduino<->ESP8266 Baudrate
#define ESP8266_BAUDRATE 115200
// FIXME?
// note: there is also a Serial object pointer definition
// in semiot-device.ino (Serial3)

// Debug Serial Baudrate
#define SERIAL_BAUDRATE 115200

#define DHT_CONNECTED //TODO
// DHT Sensor:
// AUTO_DETECT,
// DHT11,
// DHT22,
// AM2302,  // Packaged DHT22
// RHT03    // Equivalent to DHT22
// define DHTTYPE DHT22
#define DHT_DATA_PIN 2
//#define DHT_DATA_UPDATE_PERIOD 2000000 // 2 seconds, mega2560
// note: DHT_COAP_NAME defined in endpoint.h

// LED
#define LED_PIN 13
 
#endif // CONNECTIONS_H