#include <stdlib.h>
#include <coap.h> // https://github.com/1248/microcoap
#include <ESP8266.h> // https://github.com/itead/ITEADLIB_Arduino_WeeESP8266
//TODO: move to: https://github.com/niesteszeck/idDHT11
#include "DHT.h" //https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTlib

#define UDP_TX_PACKET_MAX_SIZE 860

// Wi-Fi Settings
#define SSID        "Burunduchki"
#define PASSWORD    "burunduchki"

String HOST_NAME;
#define HOST_PORT   (5683) // CoAP

// DHT 11
#define DHTTYPE DHT11   
#define DHT11_DATA_PIN 2

// Arduino<->ESP8266 Baudrate
#define BAUDRATE 9600


ESP8266 esp8266(Serial3,BAUDRATE);
DHT dht11 = DHT(DHT11_DATA_PIN,DHTTYPE);

void unregUDP()
{
    // mux_id=0:
    if (esp8266.unregisterUDP(0)) {
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
        if (esp8266.registerUDP(0,String(HOST_NAME), uint32_t(HOST_PORT)))
        {
            Serial.println("register udp ok");
            break;
        }
        else {
            Serial.println("Failed to register UDP");
            unregUDP();
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
            Serial.print("to station err\r\n");
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
            HOST_NAME = esp8266.getLocalIP();//.substring(13); // '192.168.4.1\n' -- softAP
            Serial.println(HOST_NAME);
            // Broadcast in subnet:
            // TODO: answer directly to the source:
            HOST_NAME = HOST_NAME.substring(0,HOST_NAME.lastIndexOf('.'))+String(".255");
            Serial.print("Going broadcast UDP to: ");
            Serial.println(HOST_NAME);
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
    Serial.begin(BAUDRATE);
    /*
    // tst
    Serial.println("gogo");
    char mstr[6];
    float my_number = float(20.00);
    Serial.println("My value: ");
    Serial.println(float(20.00));
    Serial.println(dtostrf(my_number,1,2,&mstr[0]));
    */
	
    Serial.print("setup...\r\n");
    Serial3.begin(BAUDRATE);
    setupESP8266();
    regUDPServer();
    regUDP();
    Serial.println("Setup status:");
    Serial.println(esp8266.getIPStatus());
    coap_setup();
    endpoint_setup();
    dht11.begin();
}

void loop()
{
    //DHT11:

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    char d = 0;
    float h = dht11.readHumidity();
    float t = dht11.readTemperature();

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
    update_dht11(&d,&h,&t);


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
            //Serial.print("size of rsplen: ");
            //Serial.println(rsplen);

            coap_packet_t rsppkt;
            coap_handle_req(&scratch_buf, &pkt, &rsppkt);

            //memset(buffer, 0, sizeof(buffer));
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
                esp8266.send(0,buffer, rsplen);
            }
        }
    }
}
