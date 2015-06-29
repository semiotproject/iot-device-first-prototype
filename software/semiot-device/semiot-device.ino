#include <stdlib.h>
#include "TimerThree.h" // https://github.com/PaulStoffregen/TimerThree


#include "ESP8266.h" // https://github.com/itead/ITEADLIB_Arduino_WeeESP8266

//TODO: move to some other lib with interruprion, maybe to this one: https://github.com/niesteszeck/idDHT11
#include "DHT.h" //https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTlib

//#define DEBUG

#include "microcoap.h"
#include "endpoints.h" // https://github.com/semiotproject/microcoap
#include "observers.h"

#define UDP_TX_PACKET_MAX_SIZE 860 // TODO: extern to 2048B?

// current client's host name and port
String HOST_NAME;
long unsigned int HOST_PORT = 5683;



#include "wifisettings.h"


HardwareSerial &esp8266_uart = Serial3; /* The UART to communicate with ESP8266 */


// #ifdef ESP8266_USE_SOFTWARE_SERIAL
// #include "SoftwareSerial.h"
// #else
// #include "HardwareSerial.h"
// #endif
//
// #ifdef ESP8266_USE_SOFTWARE_SERIAL
//     SoftwareSerial &esp8266_uart=Serial3; /* The UART to communicate with ESP8266 */
// #else
//     HardwareSerial &esp8266_uart=Serial3; /* The UART to communicate with ESP8266 */
// #endif


ESP8266 esp8266(esp8266_uart, ESP8266_BAUDRATE);
DHT dhtSensor = DHT(DHT_DATA_PIN, DHTTYPE);
char d = 0;
float h = 0;
float t = 0;

unsigned long tick=0;

String cstrToString(char* buffer, unsigned int bufferPos)
{
  String item;
  unsigned int k;
  for (int k = 0; k < bufferPos; k++) {
    item += String(buffer[k]);

  }
  return item;
}

// refresh client's host name and port
// TODO: check tetype too
void refreshClientInfo(const unsigned int mux_id)
{
  //Serial.println("refreshing...");
  String ipStatus = esp8266.getIPStatus(); // TODO: check while not avaliable
  //Serial.println(ipStatus); 
  String searchString = "+CIPSTATUS:"+String(mux_id)+",\"UDP\",\"";
  //Serial.println(searchString);
  ipStatus = ipStatus.substring(ipStatus.lastIndexOf(searchString)+searchString.length());
  String host = ipStatus.substring(0,ipStatus.indexOf("\""));
  //Serial.println(host);
  ipStatus = ipStatus.substring(host.length()+2); // ",
  String port = ipStatus.substring(0,ipStatus.indexOf(","));
  //Serial.println(port);
  //Serial.println("...");
  HOST_NAME = host;
  HOST_PORT = port.toInt();
}

// FIXME: get rid of:
/*
void unregUDP(uint8_t _mux_id)
{
  if (esp8266.unregisterUDP(_mux_id)) {
    Serial.print("unregister udp ok\r\n");
  Serial.println(esp8266.getIPStatus());
  }
  else {
    Serial.print("unregister udp err\r\n");
  }
}

void regUDP(uint8_t _mux_id, String _hostName = "", long unsigned int _port = 0)
{
  while (1) {
    //Serial.print("Trying to register UDP: ");
    //Serial.print(_mux_id,DEC);
    //Serial.print(_hostName);
    //Serial.println(_port,DEC);


    if (esp8266.registerUDP(_mux_id, _hostName, uint32_t(_port)))
    {
      Serial.println("register udp ok");
      break;
    }
    else {
      Serial.println("Failed to register UDP");
      unregUDP(_mux_id);
      Serial.println("Wait 5 seconds and try again...");
      delay(5000);
    }
  }
}

void unregUDPServer()
{
  if (esp8266.stopServer())
  {
    Serial.print("ok\r\n");
  }
  else {
    Serial.print("err\r\n");
  }
}

*/

// UDP Serv only works with mux enabled
void regUDPServer()
{
  while (1) {
    Serial.println("Trying to register UDP");
    // FIXME: hardcoded mux_id 0 and hostname
    if (esp8266.startUDPServer(0, "127.0.0.1", COAP_PORT, COAP_PORT,2))
    {
      Serial.println("ok");
      break;
    }
    else
    {
      Serial.println("Failed to start server");
      //unregUDPServer();
      Serial.println("Wait 5 seconds and try again...");
      delay(5000);
    }
  }
}

void setupESP8266()
{
  while (1)
  {
    Serial.print("restaring esp8266...");
    if (esp8266.restart())
    {
      Serial.print("ok\r\n");
      break;
    }
    else
    {
      Serial.print("not ok...\r\n");
      Serial.print("Trying to kick...");
      while (1) {
        if (esp8266.kick())
        {
          Serial.print("ok\r\n");
          break;
        }
        else
        {
          Serial.print("not ok... Wait 5 sec and retry...\r\n");
          delay(5000);
        }
      }
    }
  }

  Serial.print("setup begin\r\n");

  Serial.print("FW Version:");
  Serial.println(esp8266.getVersion().c_str());

  while (1)
  {
    if (esp8266.setOprToStation())
    {
      Serial.print("to station ok\r\n");
      break;
    }
    else
    {
      Serial.print("to stati1on err\r\n");
      Serial.println("Wait 5 seconds and try again...");
      delay(5000);
    }
  }

  while (1)
  {
    if (esp8266.joinAP(SSID, PASSWORD))
    {
      Serial.print("Join AP success\r\n");
      Serial.print("IP: ");
      Serial.println(esp8266.getLocalIP());//.substring(13); // '192.168.4.1\n' -- softAP
      break;
    }
    else
    {
      Serial.print("Join AP failure\r\n");
      Serial.println("Wait 5 seconds and try again...");
      delay(5000);
    }
  }

  Serial.print("Trying to enable mux:");
  while (1)
  {
    if (esp8266.enableMUX())
    {
      Serial.print("enable mux ok\r\n");
      break;

    }
    else
    {
      Serial.print("enable mux err\r\n");
      Serial.println("Wait 5 seconds and try again...");
      delay(5000);
    }
  }

  Serial.print("setup end\r\n");
}


//DHT Sensor
void refreshDHTSensor()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  d = 0;
  h = dhtSensor.readHumidity();
  t = dhtSensor.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h))
  {
    d = 0;
    h = 0;
    t = 0;
    //Serial.println("Failed to read from DHT");
  }
  else
  {
    d = 1;
    /*
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
    */
  }
  tick+=1;
}


void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  Serial.print("setup...\r\n");
  Serial3.begin(ESP8266_BAUDRATE);
  setupESP8266();
  regUDPServer();
  Serial.println("Setup status:");
  Serial.println(esp8266.getIPStatus());
  coap_setup();
  endpoint_setup();
  setup_dht_endpoint(&d, &h, &t);
  dhtSensor.begin();
  refreshDHTSensor();
  Timer3.initialize(DHT_DATA_UPDATE_PERIOD);         // initialize timer3, and set a 2 seconds period
  Timer3.attachInterrupt(refreshDHTSensor);  // attaches refreshDHTSensor() as a timer overflow interrupt
  Serial.println("ready");
}

//-----------COAP with observe:

coap_packet_t pkt; // parse recieved coap packet
uint8_t buffer[UDP_TX_PACKET_MAX_SIZE] = {0}; // recieve and send buffer
uint32_t len; // esp8266.recv
int rc; // coap error codes


coap_packet_t rsppkt;
static uint8_t scratch_raw[UDP_TX_PACKET_MAX_SIZE];
static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};
//size_t rsplen; //TODO:


void sendCoAPpkt(coap_packet_t* pkt_p, String hostName = "", long unsigned int port = 0, bool addTick=false)
{
    size_t rsplen = sizeof(buffer);

  if (!addTick) {
      coap_handle_req(&scratch_buf, pkt_p, &rsppkt);
  }
  else {
      /*
      unsigned long tick;
    // http://tools.ietf.org/html/draft-ietf-core-observe-16#section-4.4
    tick = micros()/30.52;
    */
    //TESTME:
    Serial.print("tick");
    const uint8_t p = tick;
    Serial.println(p);
    Serial.println(rsppkt.opts[0].num);
    rsppkt.opts[0].buf.p = &p;
    rsppkt.opts[0].buf.len = sizeof(p);
    memcpy(&rsppkt, pkt_p, sizeof(pkt_p));
  }
  
  if (0 != (rc = coap_build(buffer, &rsplen, &rsppkt)))
  {
    Serial.print("coap_build failed rc=");
    Serial.println(rc, DEC);
  }
  else
  {
      /*
    Serial.print("Answer ready:[");
    for (uint32_t i = 0; i < rsplen; i++)
    {
      Serial.print(buffer[i], HEX);
      Serial.print(' ');
    }
    Serial.print("]\r\n");
    Serial.print("sending to: ");
    Serial.print(hostName);
    Serial.print(":");
    Serial.println(port);
    */
    
    esp8266.send(0, buffer, rsplen);
    Serial.println("Answer sended");
  }
}

bool coapSubscribe(coap_packet_t* pkt_p, String hostName = "", long unsigned int port = 0)
{
  size_t rsplen = sizeof(buffer);
  coap_handle_req(&scratch_buf, pkt_p, &rsppkt);

  rsppkt.numopts=2;
      
    rsppkt.opts[1].num=rsppkt.opts[0].num;
    rsppkt.opts[1].buf.p = rsppkt.opts[0].buf.p;
    rsppkt.opts[1].buf.len = rsppkt.opts[0].buf.len;

    rsppkt.opts[0].num = COAP_OPTION_OBSERVE;
    
    //FIXME:
    Serial.print("tick");
    const uint8_t p = tick;
    Serial.println(p);
    Serial.println(rsppkt.opts[0].num);
    rsppkt.opts[0].buf.p = &p;
    rsppkt.opts[0].buf.len = sizeof(p);

  if (0 != (rc = coap_build(buffer, &rsplen, &rsppkt)))
  {
    Serial.print("coap_build failed rc=");
    Serial.println(rc, DEC);
  }
  else
  {
      /*
    Serial.print("Answer ready:[");
    for (uint32_t i = 0; i < rsplen; i++)
    {
      Serial.print(buffer[i], HEX);
      Serial.print(' ');
    }
    Serial.print("]\r\n");

    Serial.print("sending to: ");
    Serial.print(hostName);
    Serial.print(":");
    Serial.println(port);
    */
    
    esp8266.send(0, buffer, rsplen);
    Serial.println("Answer sended");
    rsppkt.hdr.t = COAP_TYPE_NONCON;
    addCoAPObserver(HOST_NAME.c_str(), HOST_NAME.length(), HOST_PORT, rsppkt);
    Serial.println("Oberver added"); //TODO
    return true;
  }
  return false;
    
}


void listenCoAP()
{
    len = esp8266.recv(buffer, sizeof(buffer), 1000);
  
  if (len > 0)
  {
    Serial.print("Received:[");
    for (uint32_t i = 0; i < len; i++)
    {
      Serial.print(buffer[i], HEX);
      Serial.print(' ');
    }
    Serial.print("]\r\n");

    // prepare coap answer:

    if (0 != (rc = coap_parse(&pkt, buffer, len)))
    {
      Serial.print("Bad packet rc=");
      Serial.println(rc, DEC);
    }
    else
    {
      // FIXME: hardcoded 0 mux_id
      refreshClientInfo(0);
      
      // cheking for observe option:
      uint8_t optionNumber;
      coap_buffer_t* val = NULL;
      for (optionNumber = 0; optionNumber < pkt.numopts; optionNumber++)
      {
        coap_option_t* currentOption = &pkt.opts[optionNumber];
        if (currentOption->num == COAP_OPTION_OBSERVE)
        {
          val = &currentOption->buf;
          break;
        }
      }
      if (val != NULL) {
        // http://tools.ietf.org/html/draft-ietf-core-observe-16#section-2
        if (val->len == 0) { // register
          coapSubscribe(&pkt, HOST_NAME, HOST_PORT);
          Serial.println("subscribe ok"); // TODO: add if
        }
        //TODO: unsubscribe
      }
      else {
        sendCoAPpkt(&pkt, HOST_NAME, HOST_PORT);
      }
    }
  }
}

void sendToObservers()
{
    // TODO: only if the res really changed
  int i;
  if (getObserversCount() > 0)
  {
      Serial.print("Observers count: ");
      Serial.println(getObserversCount(),DEC);
    for (i = 0; i < getObserversCount(); i++) {
        Serial.print("sending to observer: ");
        Serial.print(cstrToString(observers[i].hostName, observers[i].hostNameLenght));
        Serial.print(":");
        Serial.println(observers[i].port, DEC);
      sendCoAPpkt(&observers[i].pkt, cstrToString(observers[i].hostName, observers[i].hostNameLenght), observers[i].port,true);
    }
  }
}

void loop()
{
  listenCoAP();
  sendToObservers();
}
