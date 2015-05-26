#include <stdlib.h>
#include "endpoints.h"
#include "coap.h" // https://github.com/1248/microcoap
#include "ESP8266.h" // https://github.com/itead/ITEADLIB_Arduino_WeeESP8266
//TODO: move to: https://github.com/niesteszeck/idDHT11
#include "DHT.h" //https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTlib

#define UDP_TX_PACKET_MAX_SIZE 860 // TODO: extern to 2048B?

#include "semiotsettings.h"

String HOST_NAME;

ESP8266 esp8266(esp8266_uart,ESP8266_BAUDRATE);
DHT dht = DHT(DHT_DATA_PIN,DHTTYPE);

void unregUDP()
{
    // mux_id=0:
    if (esp8266.unregisterUDP(3)) {
        Serial.print("unregister udp ok\r\n");
    }
    else {
        Serial.print("unregister udp err\r\n");
    }
}

void regUDP()
{
    // mux_id=0:
    while (1) {
        Serial.println("Trying to register UDP");
        // STATUS:N\n"+CIPSTATUS:N,"UDP","192.168.43.178",5683,N"
        // HOST_NAME.substring(0,HOST_NAME.lastIndexOf('.'))+String(".255");
        HOST_NAME = esp8266.getIPStatus();
        //Serial.println(HOST_NAME);
        HOST_NAME = HOST_NAME.substring(30); // STATUS:N\n"+CIPSTATUS:N,"UDP","
        //Serial.println(HOST_NAME);
        HOST_PORT = HOST_NAME.substring(HOST_NAME.indexOf(',')+1,HOST_NAME.lastIndexOf(',')).toInt();
        //Serial.println("");
        //Serial.println(HOST_NAME);
        //Serial.println(HOST_PORT);
        //Serial.println("");
        HOST_NAME = HOST_NAME.substring(0,HOST_NAME.indexOf('"'));
        
        Serial.println(HOST_NAME);
        Serial.println(HOST_PORT);

        
        if (esp8266.registerUDP(3,HOST_NAME, uint32_t(HOST_PORT)))
        {
            Serial.println("register udp ok");
            break;
        }
        else {
            Serial.println("Failed to register UDP");
            //unregUDP();
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
        if (esp8266.startServer(uint32_t(HOST_PORT)))
        {
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


void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.print("setup...\r\n");
    Serial3.begin(ESP8266_BAUDRATE);
    setupESP8266();
    regUDPServer();
    //regUDP();
    Serial.println("Setup status:");
    Serial.println(esp8266.getIPStatus());
    coap_setup();
    endpoint_setup();
    dht.begin();
}

void loop()
{
    //DHT11:

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    char d = 0;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

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


    //CoAP:

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
        static uint8_t scratch_raw[UDP_TX_PACKET_MAX_SIZE];
        static coap_rw_buffer_t scratch_buf = {scratch_raw, sizeof(scratch_raw)};

        if (0 != (rc = coap_parse(&pkt, buffer, len)))
        {
            Serial.print("Bad packet rc=");
            Serial.println(rc, DEC);
        }
        else
        {
            size_t rsplen = sizeof(buffer);

            coap_packet_t rsppkt;
            coap_handle_req(&scratch_buf, &pkt, &rsppkt);

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
                regUDP();
                esp8266.send(0,buffer, rsplen);
                //unregUDP();
            }
        }
    }
}
