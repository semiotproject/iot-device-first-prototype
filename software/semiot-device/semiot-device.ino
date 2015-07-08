#include <stdlib.h>
#include "TimerThree.h" // https://github.com/PaulStoffregen/TimerThree


#include "ESP8266.h" // https://github.com/itead/ITEADLIB_Arduino_WeeESP8266

#include "DHT.h" // http://www.github.com/markruys/arduino-DHT

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
uint8_t udp_mux_id=0; // esp8266 mux_id using for udp connections
DHT dhtSensor;
char d = 0;
float h = 0;
float t = 0;
bool timeToUpdate=false;
bool d_changed=false;
bool h_changed=false;
bool t_changed=false;

uint8_t tick=0;

// FIXME: get rid of Strings:
String cstrToString(char* buffer, unsigned int bufferPos)
{
    String item;
    unsigned int k;
    for (int k = 0; k < bufferPos; k++) {
        item += String(buffer[k]);
    }
    return item;
}

//TODO: move to microcoap.h:
// saving the uri path from pkt_p:
void parse_uri_path_opt(coap_packet_t* pkt_p, coap_endpoint_path_t* dest) {
    unsigned int opt_count = pkt_p->numopts;
    unsigned int segm_count = 0;
    unsigned int opt_i;
    int segment_lengh=0;
    for (opt_i=0;opt_i<opt_count;opt_i++) {
        if (pkt_p->opts[opt_i].num==COAP_OPTION_URI_PATH) {
            segment_lengh = pkt_p->opts[opt_i].buf.len;
            //Serial.print("segment_lengh: ");
            //Serial.println(segment_lengh,DEC);
            int buflen = pkt_p->opts[opt_i].buf.len;
            const uint8_t* buf = pkt_p->opts[opt_i].buf.p;
            char segment[buflen];
            int segm_i = 0;
            while(buflen--) {
                uint8_t x = *buf++;
                char c = char(x);
                //Serial.print(char(c));
                //Serial.print(" ");
                segment[segm_i]=c;
                segm_i++;
            }
            //Serial.print("\n");
            //FIXME: get rid of String:
            String string = cstrToString(segment,segment_lengh);
            dest->elems[segm_count]=(char*)calloc(segment_lengh+1,sizeof(char));
            strncpy((char *)dest->elems[segm_count], string.c_str(), segment_lengh);
            segm_count+=1;
        }
    }
    dest->count=segm_count;
}

// refresh client's host name and port
// TODO: check tetype too
void refreshClientInfo(const unsigned int mux_id)
{
  String ipStatus = esp8266.getIPStatus(); // TODO: check while not avaliable
  String searchString = "+CIPSTATUS:"+String(mux_id)+",\"UDP\",\"";
  ipStatus = ipStatus.substring(ipStatus.lastIndexOf(searchString)+searchString.length());
  String host = ipStatus.substring(0,ipStatus.indexOf("\""));
  ipStatus = ipStatus.substring(host.length()+2); // ",
  String port = ipStatus.substring(0,ipStatus.indexOf(","));
  HOST_NAME = host;
  HOST_PORT = port.toInt();
}

// UDP Serv only works with mux enabled
void regUDP()
{
    while (1) {
        Serial.println("Trying to register UDP");
        // 127.0.0.1 is temporary until new udp packet
        // beause udp mode = 2
        if (esp8266.registerUDP(udp_mux_id, "127.0.0.1", COAP_PORT, COAP_PORT,2)) {
            Serial.println("ok");
            break;
        }
        else {
            Serial.println("Failed to start UDP");
            Serial.println("Wait 5 seconds and try again...");
            delay(5000);
        }
    }
}

void setupESP8266()
{
    while (1) {
        Serial.print("restaring esp8266...");
        if (esp8266.restart()) {
            Serial.print("ok\r\n");
            break;
        }
        else {
            Serial.print("not ok...\r\n");
            Serial.print("Trying to kick...");
            while (1) {
                if (esp8266.kick()) {
                    Serial.print("ok\r\n");
                    break;
                }
                else {
                    Serial.print("not ok... Wait 5 sec and retry...\r\n");
                    delay(5000);
                }
            }
        }
    }

    Serial.print("setup begin\r\n");
    Serial.print("FW Version:");
    Serial.println(esp8266.getVersion().c_str());

    while (1) {
        if (esp8266.setOprToStation()) {
            Serial.print("to station ok\r\n");
            break;
        }
        else {
            Serial.print("to stati1on err\r\n");
            Serial.println("Wait 5 seconds and try again...");
            delay(5000);
        }
    }

    while (1) {
        if (esp8266.joinAP(SSID, PASSWORD)) {
            Serial.print("Join AP success\r\n");
            Serial.print("IP: ");
            Serial.println(esp8266.getLocalIP());
            break;
        }
        else {
            Serial.print("Join AP failure\r\n");
            Serial.println("Wait 5 seconds and try again...");
            delay(5000);
        }
    }

    Serial.print("Trying to enable mux:");
    while (1) {
        if (esp8266.enableMUX()) {
            Serial.print("enable mux ok\r\n");
            break;
        }
        else {
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
    if (timeToUpdate) {
        String d_cur = (String)dhtSensor.getStatusString();
        if (d_cur=="TIMEOUT") {
            if (d!=1) {
                d=1;
                d_changed=true;
            }
        }
        if (d_cur == "CHECKSUM") {
        if (d!=2) {
                d=2;
                d_changed=true;
            }
        }
        if (!d_changed) {
        if (d!=0) {
                d=0;
                d_changed=true;
            }
        }
        // TODO: if d=0
        float h_cur = dhtSensor.getHumidity();
        if (h!=h_cur) {
            h=h_cur;
            h_changed=true;
        }
        float t_cur = dhtSensor.getTemperature();
        if (t!=t_cur) {
            t=t_cur;
            t_changed=true;
        }
        tick+=1;
        setup_dht_endpoint(&d, &h, &t); //
        timeToUpdate=false;
    }
}

void updateTime()
{
    timeToUpdate = true;
}


void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Serial.print("setup...\r\n");
    Serial3.begin(ESP8266_BAUDRATE);
    setupESP8266();
    regUDP();
    Serial.println("Setup status:");
    Serial.println(esp8266.getIPStatus());
    coap_setup();
    endpoint_setup();
    setup_dht_endpoint(&d, &h, &t);
    dhtSensor.setup(DHT_DATA_PIN);
    unsigned long period = dhtSensor.getMinimumSamplingPeriod()*1000;
    Serial.print("Setting timer period, microseconds: ");
    Serial.println(period,DEC);
    // FIXME: save this period to timer:
    Timer3.initialize(2000000); // initialize timer3, and set a 2 seconds period
    Timer3.attachInterrupt(updateTime); // attaches function as a timer overflow interrupt
    Serial.println("ready");
}

// COAP with observe:

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
        rsppkt.opts[0].buf.p = &tick;
        rsppkt.opts[0].buf.len = sizeof(tick);
        rsppkt.hdr.id[0]++;
        rsppkt.hdr.id[1]++;
        memcpy(&rsppkt, pkt_p, sizeof(pkt_p));
    }
    if (0 != (rc = coap_build(buffer, &rsplen, &rsppkt))) {
        Serial.print("coap_build failed rc=");
        Serial.println(rc, DEC);
    }
    else {
        esp8266.send(udp_mux_id, buffer, rsplen);
        Serial.println("Answer sended");
    }
}

bool coapUnsubscribe(coap_packet_t* pkt_p, String* hostName, long unsigned int* port) {
    coap_endpoint_path_t uri_path;
    parse_uri_path_opt(pkt_p,&uri_path);
    return removeCoApObserver(hostName->c_str(), hostName->length(), port, &uri_path);
}

bool coapSubscribe(coap_packet_t* pkt_p, String* hostName, long unsigned int* port)
{
    /*
    Serial.println("dump:");
                int buflen = pkt.opts[1].buf.len;
                const uint8_t* buf = pkt.opts[1].buf.p;
                char segment[buflen];
                int segm_i = 0;
                while(buflen--) {
                    uint8_t x = *buf++;
                    
                    char c = char(x);
                    Serial.print(char(c));
                    Serial.print(" ");
                    segment[segm_i]=c;
                    segm_i++;
                }
                Serial.print("\n");
    */
    // saving uri_path before corrupting the pkt:            
    coap_endpoint_path_t uri_path;
    parse_uri_path_opt(pkt_p,&uri_path);
    
    //handling request:
    size_t rsplen = sizeof(buffer);
    coap_handle_req(&scratch_buf, pkt_p, &rsppkt);
    rsppkt.numopts=2;
    rsppkt.opts[1].num=rsppkt.opts[0].num;
    rsppkt.opts[1].buf.p = rsppkt.opts[0].buf.p;
    rsppkt.opts[1].buf.len = rsppkt.opts[0].buf.len;
    rsppkt.opts[0].num = COAP_OPTION_OBSERVE;
    // FIXME: separate add tick function
    rsppkt.opts[0].buf.p = &tick;
    rsppkt.opts[0].buf.len = sizeof(tick);
    if (0 != (rc = coap_build(buffer, &rsplen, &rsppkt))) {
        Serial.print("coap_build failed rc=");
        Serial.println(rc, DEC);
    }
    else {
        esp8266.send(0, buffer, rsplen);
        Serial.println("Answer sended");
        tick++;
        rsppkt.hdr.t = COAP_TYPE_NONCON;
        /*
        Serial.println("uri path: ");
        Serial.println(uri_path.elems[0]);
        Serial.println(uri_path.elems[1]);
        */
        if (addCoAPObserver(hostName->c_str(), hostName->length(), port, rsppkt,&uri_path)) {
            Serial.println("Oberver added");
        }
        return true;
    }
    return false;
}

// use this functions:

void listenCoAP()
{
    len = esp8266.recv(buffer, sizeof(buffer), 1000);
    if (len > 0) {
        // prepare coap answer:
        if (0 != (rc = coap_parse(&pkt, buffer, len))) {
            Serial.print("Bad packet rc=");
            Serial.println(rc, DEC);
        }
        else {
            refreshClientInfo(udp_mux_id);
            // FIXME: BAD: UGLY: REWRITE!
            int o;
            /*
            Serial.println("println:");
            for (o=1;o<=2;o++) {
                //uri_path.count=2;
                Serial.println((char*)pkt.opts[o].buf.p);
                //uri_path.elems[o-1]=cstrToString((char*)pkt_p.opts[o].buf.p,pkt_p.opts[o].buf.len).c_str();  
            }
            */
            /*
            Serial.println("dump:");
                int buflen = pkt.opts[1].buf.len;
                const uint8_t* buf = pkt.opts[1].buf.p;
                char segment[buflen];
                int segm_i = 0;
                while(buflen--) {
                    uint8_t x = *buf++;
                    
                    char c = char(x);
                    Serial.print(char(c));
                    Serial.print(" ");
                    segment[segm_i]=c;
                    segm_i++;
                }
                Serial.print("\n");
            */
            
            // cheking for observe option:
            uint8_t optionNumber;
            coap_buffer_t* val = NULL;
            for (optionNumber = 0; optionNumber < pkt.numopts; optionNumber++) {
                coap_option_t* currentOption = &pkt.opts[optionNumber];
                if (currentOption->num == COAP_OPTION_OBSERVE) {
                    val = &currentOption->buf;
                    break;
                }
            }
            if (val != NULL) {
            // http://tools.ietf.org/html/draft-ietf-core-observe-16#section-2
                if (val->len == 0) { // register
                    if (coapSubscribe(&pkt, &HOST_NAME, &HOST_PORT)) {
                        Serial.println("subscribe ok");
                    }
                }
                else {
                    if ((val->len == 1) && (*val->p==1)) {
                        //TODO:
                        if (coapUnsubscribe(&pkt,&HOST_NAME,&HOST_PORT)) {
                            Serial.println("unsubscribe ok");
                        }
                    }
                }
            }
            else {
                sendCoAPpkt(&pkt, HOST_NAME, HOST_PORT);
            }
        }
    }
}

/*
//FIXME:
bool is_coap_endpoint_path_t_eq(const coap_endpoint_path_t* a, const coap_endpoint_path_t* b) {
    bool eq = false;
    unsigned int count = a->count;
    
    if (count==b->count) {
        unsigned int i;
        for (i=0;i<count;i++) {
            if (strcmp(a->elems[i], b->elems[i]) != 0) {
                return eq;
            }
        }
        eq = true;
        return eq;
    }
    return eq;
}
*/


void sendToObservers()
{
    int i;
    if (getObserversCount() > 0)
    {
        //Serial.print("Observers count: ");
        //Serial.println(getObserversCount(),DEC);
        for (i = 0; i < getObserversCount(); i++) {
            //Serial.print("sending to observer: ");
            //Serial.print(cstrToString(observers[i].hostName, observers[i].hostNameLenght));
            //Serial.print(":");
            //Serial.println(observers[i].port, DEC);
            bool should_send=false;
            unsigned int n = observers[i].path.count;
            unsigned n_i;
            if ((d_changed) && is_coap_endpoint_path_t_eq(&observers[i].path,&path_dht)) {
                should_send=true;
                d_changed=false;
            }
            if ((h_changed) && is_coap_endpoint_path_t_eq(&observers[i].path,&path_dht_humidity)) {
                should_send=true;
                h_changed=false;
            }
            if ((t_changed) && is_coap_endpoint_path_t_eq(&observers[i].path,&path_dht_temperature)) {
                should_send=true;
                t_changed=false;
            }
            //Serial.println(should_send, DEC);
            //Serial.println(h, DEC);
            if (should_send) {
                sendCoAPpkt(&observers[i].answer_draft_pkt, cstrToString(observers[i].hostName, observers[i].hostNameLenght), observers[i].port,true);
            }
        }
    }
}

void loop()
{
    //TODO: add observers checker;
    refreshDHTSensor(); // refreshing only when interred minimum sampling period is passed
    listenCoAP(); // listen always;
    sendToObservers(); // sending only when apropriate data is changed
}
