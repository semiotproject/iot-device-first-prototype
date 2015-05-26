// Debug Serial Baudrate
#define SERIAL_BAUDRATE 9600

// Arduino<->ESP8266 Baudrate
HardwareSerial &esp8266_uart=Serial3;
#define ESP8266_BAUDRATE 9600

// esp8266
#define SSID "SSID"
#define PASSWORD "PASSWORD"

long unsigned int HOST_PORT=5683; // CoAP


// DHT Sensor
// DHT11 11
// DHT22 22
// DHT21 21
// AM2301 21
#define DHT_CONNECTED
#define DHTTYPE DHT11   
#define DHT_DATA_PIN 2


