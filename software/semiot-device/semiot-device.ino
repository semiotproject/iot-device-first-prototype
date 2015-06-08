#include <stdlib.h>
#include "TimerThree.h" // https://github.com/PaulStoffregen/TimerThree


#include "ESP8266.h" // https://github.com/itead/ITEADLIB_Arduino_WeeESP8266

//TODO: move to some other lib with interruprion, maybe to this one: https://github.com/niesteszeck/idDHT11
#include "DHT.h" //https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTlib

#include "microcoap.h" 
#include "endpoints.h" // https://github.com/semiotproject/microcoap
#include "observers.h"


#define UDP_TX_PACKET_MAX_SIZE 860 // TODO: extern to 2048B?

// current client's host name and port
String HOST_NAME;
long unsigned int COAP_PORT=5683;



#include "wifisettings.h"


HardwareSerial &esp8266_uart=Serial3; /* The UART to communicate with ESP8266 */


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


ESP8266 esp8266(esp8266_uart,ESP8266_BAUDRATE);
DHT dhtSensor = DHT(DHT_DATA_PIN,DHTTYPE);

String cstrtToString(char* buffer, unsigned int bufferPos)
{
    String item;
    unsigned int k;
    for(int k=0; k<bufferPos; k++){
        item += String(buffer[k]);
        
    }
    return item;
}

// refresh client's host name and port
void refreshClientInfo()
{
    // TODO:
    if (0) {
        
    String hostName;
    long unsigned int port;
    Serial.println("new refreshing info:");
    // STATUS:N\n"+CIPSTATUS:N,"UDP","192.168.43.178",5683,N"
    // HOST_NAME.substring(0,HOST_NAME.lastIndexOf('.'))+String(".255");
    hostName = esp8266.getIPStatus();
    Serial.println(hostName);
    //Serial.println(HOST_NAME);
    hostName = hostName.substring(30); // STATUS:N\n"+CIPSTATUS:N,"UDP","
    //Serial.println(HOST_NAME);
    port = HOST_NAME.substring(HOST_NAME.indexOf(',')+1,HOST_NAME.lastIndexOf(',')).toInt();
    //Serial.println("");
    //Serial.println(HOST_NAME);
    //Serial.println(COAP_PORT);
    //Serial.println("");
    hostName = hostName.substring(0,HOST_NAME.indexOf('"'));
    Serial.println(hostName);
    Serial.println(port);
    Serial.println("");
    HOST_NAME=hostName;
    COAP_PORT=port;
    
    }
}

void unregUDP(uint8_t _mux_id)
{
    if (esp8266.unregisterUDP(_mux_id)) {
        Serial.print("unregister udp ok\r\n");
    }
    else {
        Serial.print("unregister udp err\r\n");
    }
}

void regUDP(uint8_t _mux_id, String _hostName="",long unsigned int _port=0)
{
    while (1) {
        Serial.println("Trying to register UDP");
        
        Serial.println(HOST_NAME);
        Serial.println(COAP_PORT);

        
        if (esp8266.registerUDP(_mux_id,_hostName, uint32_t(_port)))
        {
            Serial.println("register udp ok");
            break;
        }
        else {
            Serial.println("Failed to register UDP");
            //unregUDP(_mux_id); // FIXME: firmware bug?
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


// We need it to recieve the data
// UDP Serv only works with mux enabled
// esp8266 firmware: 0.9.2.4
void regUDPServer()
{
    while (1) {
        Serial.println("Try to start server");
        if (esp8266.startServer(uint32_t(COAP_PORT)))
        {
            // FIXME: hardcoded mux_id
            regUDP(0,String("192.168.1.14"),COAP_PORT);
            Serial.println("ok");
            break;
        }
        else
        {
            Serial.println("Failed to start server");
            unregUDPServer();
            Serial.println("Wait 5 seconds and try again...");
            delay(5000);
        }
    }
}

void setupESP8266()
{
    while(1)
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
            while(1) {
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
    while(1)
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

void sendCoAPpkt(uint8_t mux_id, coap_packet_t* pkt_p, String hostName="",long unsigned int port=0)
{
    int rc;
            
    uint8_t buffer[UDP_TX_PACKET_MAX_SIZE] = {0}; // recieve and send buffer
    static uint8_t scratch_raw[UDP_TX_PACKET_MAX_SIZE];
static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};
    
    size_t rsplen = sizeof(buffer);

    coap_packet_t rsppkt;
    coap_handle_req(&scratch_buf, pkt_p, &rsppkt);

    if (0 != (rc = coap_build(buffer, &rsplen, &rsppkt)))
    {
        Serial.print("coap_build failed rc=");
        Serial.println(rc, DEC);
    }
    else
    {
        Serial.print("Answer ready:[");
        for(uint32_t i = 0; i < rsplen; i++)
        {
            Serial.print(buffer[i],HEX);
            Serial.print(' ');
        }
        Serial.print("]\r\n");
        regUDP(mux_id,hostName,port);
        esp8266.send(3,buffer, rsplen);
        // unregUDP(mux_id,hostName,port); -- FIXME: firmware bug?
    }
    
}

void sendToObservers()
{
    int i;
    if (observersCount>0)
    {
        for (i=0;i<observersCount;i++) {
            // FIXME: hardcoded 4th mux_id
            sendCoAPpkt(4,&observers[i].pkt,cstrtToString(observers[i].hostName,observers[i].hostNameLenght),observers[i].port);        
        }
    }
}

//DHT Sensor
void refreshDHTSensor()
{
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    char d = 0;
    float h = dhtSensor.readHumidity();
    float t = dhtSensor.readTemperature();

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h))
    {
        d = 0;
        h = 0;
        t = 0;
        Serial.println("Failed to read from DHT");
    }
    else
    {
        d = 1;
        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" *C");
    }
    update_dht(&d,&h,&t);
    
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
    dhtSensor.begin();
    refreshDHTSensor();
    Timer3.initialize(DHT_DATA_UPDATE_PERIOD);         // initialize timer3, and set a 2 seconds period
    Timer3.attachInterrupt(refreshDHTSensor);  // attaches refreshDHTSensor() as a timer overflow interrupt
}

void refreshCoAP()
{
    uint8_t buffer[UDP_TX_PACKET_MAX_SIZE] = {0}; // recieve and send buffer

    uint32_t len = esp8266.recv(buffer, sizeof(buffer), 1000);
    if (len > 0)
    {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++)
        {
            Serial.print(buffer[i],HEX);
            Serial.print(' ');
        }
        Serial.print("]\r\n");

        // prepare coap answer:

        int rc;
        coap_packet_t pkt;

        if (0 != (rc = coap_parse(&pkt, buffer, len)))
        {
            Serial.print("Bad packet rc=");
            Serial.println(rc, DEC);
        }
        else
        {
            refreshClientInfo();
            // cheking for observe:
            int optionNumber;
            
            coap_buffer_t* val = NULL;
            
            for (optionNumber=0;optionNumber<pkt.numopts;optionNumber++)
            {
                coap_option_t* currentOption = &pkt.opts[optionNumber];
                if (currentOption->num==6)
                {
                    val = &currentOption->buf;
                    break;
                }
            }
            
            if (val!=NULL) {
                // http://tools.ietf.org/html/draft-ietf-core-observe-16#section-2
                // FIXME: bad comparison types:
                if (*val->p==0) { // register
                    addCoAPObserver(HOST_NAME.c_str(),HOST_NAME.length(),COAP_PORT,pkt);
                    Serial.print("subscribe ok");
                }
                if (*val->p==1) { // deregister
                }
                
            }
            
            // FIXME: hardcoded 3 mux_id
            sendCoAPpkt(3, &pkt,HOST_NAME,COAP_PORT);
        }
    }
}

void loop()
{
    refreshCoAP();
}
