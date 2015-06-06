#include <stdbool.h>
#include <string.h>

#include "microcoap.h"

#include "connections.h"
#include "coapsettings.h"
#include "wifisettings.h"

// TODO: move to CoAP settings:
#define OBS_RES_MAX 1 // max number of the observed resourses
#define DHT_STRING_SIZE 6

static char dht_t[DHT_STRING_SIZE];
static char dht_h[DHT_STRING_SIZE];
static char light = '0';
static char* dht;
static char* dht_temperature;
static char* dht_humidity;

coap_endpoint_path_t obs_path_list[OBS_RES_MAX];

#define RSP_BUFFER_SIZE 108//64
const uint16_t rsplen = RSP_BUFFER_SIZE;
static char rsp[RSP_BUFFER_SIZE] = "";
void build_rsp(void);

#ifdef ARDUINO
#include "Arduino.h"
void endpoint_setup(void)
{                
    pinMode(LED_PIN, OUTPUT);     
    build_rsp();
}
#else
#include <stdio.h>
void endpoint_setup(void)
{
    build_rsp();
}
#endif

static const coap_endpoint_path_t path_well_known_core = {2, {".well-known", "core"}};
static int handle_get_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static const coap_endpoint_path_t path_light = {1, {"light"}};
static int handle_get_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    if (inpkt->payload.len == 0)
        return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
    if (inpkt->payload.p[0] == '1')
    {
        light = '1';
#ifdef ARDUINO
        digitalWrite(LED_PIN, HIGH);
#else
        printf("ON\n");
#endif
        return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
    else
    {
        light = '0';
#ifdef ARDUINO
        digitalWrite(LED_PIN, LOW);
#else
        printf("OFF\n");
#endif
        return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
    }
}

static const coap_endpoint_path_t path_dht = {1, {DHT_COAP_NAME}};
static const coap_endpoint_path_t path_dht_temperature = {2,  {DHT_COAP_NAME, "temperature"}};
static const coap_endpoint_path_t path_dht_humidity = {2,  {DHT_COAP_NAME, "humidity"}};

static int handle_get_dht(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)dht, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_dht_temperature(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const uint8_t *)dht_temperature, DHT_STRING_SIZE, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_get_dht_humidity(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
    return coap_make_response(scratch, outpkt, (const char *)dht_humidity, DHT_STRING_SIZE, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core, "ct=40"},

    {COAP_METHOD_GET, handle_get_light, &path_light, "ct=0"},
    {COAP_METHOD_PUT, handle_put_light, &path_light, NULL},

    {COAP_METHOD_GET, handle_get_dht, &path_dht, "ct=0"},
    {COAP_METHOD_GET, handle_get_dht_temperature, &path_dht_temperature, "ct=0"},

    {COAP_METHOD_GET, handle_get_dht_humidity, &path_dht_humidity, "ct=0"},


    {(coap_method_t)0, NULL, NULL, NULL}
};

void update_dht(char* dht_avaliable, float* humidity, float* temperature)
{
    dht=dht_avaliable;
    dht_temperature=dtostrf(*temperature,1,2,&dht_t[0]);
    dht_humidity=dtostrf(*humidity,1,2,&dht_h[0]);
}


void build_rsp(void)
{
    uint16_t len = rsplen;
    const coap_endpoint_t *ep = endpoints;
    int i;

    len--; // Null-terminated string

    while(NULL != ep->handler)
    {
        if (NULL == ep->core_attr) {
            ep++;
            continue;
        }

        if (0 < strlen(rsp)) {
            strncat(rsp, ",", len);
            len--;
        }

        strncat(rsp, "<", len);
        len--;

        for (i = 0; i < ep->path->count; i++) {
            strncat(rsp, "/", len);
            len--;

            strncat(rsp, ep->path->elems[i], len);
            len -= strlen(ep->path->elems[i]);
        }

        strncat(rsp, ">;", len);
        len -= 2;

        strncat(rsp, ep->core_attr, len);
        len -= strlen(ep->core_attr);

        ep++;
    }
}

