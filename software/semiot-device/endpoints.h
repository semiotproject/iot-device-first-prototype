#ifndef   ENDPOINTS_H
#define   ENDPOINTS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>

#include "connections.h"
#include "microcoap.h"

#define DHT_STRING_SIZE 6

#define DHT_COAP_NAME "dht22"
#define MAX_SEGMENTS 2  // 2 = /foo/bar, 3 = /foo/bar/baz // FIXME: get rid of microcoap.h definition
    
static char dht_t_str[DHT_STRING_SIZE];
static char dht_h_str[DHT_STRING_SIZE];
static char light = '0';
static char* dht;
static char* dht_temperature;
static char* dht_humidity;

#define RSP_BUFFER_SIZE 108//64
const uint16_t rsplen = RSP_BUFFER_SIZE;
static char rsp[RSP_BUFFER_SIZE] = "";
void build_rsp(void);

void endpoint_setup(void);

static const coap_endpoint_path_t path_well_known_core = {2, {".well-known", "core"}};
static int handle_get_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

static const coap_endpoint_path_t path_light = {1, {"light"}};
static int handle_get_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

static int handle_put_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

static const coap_endpoint_path_t path_dht = {1, {DHT_COAP_NAME}};
static const coap_endpoint_path_t path_dht_temperature = {2,  {DHT_COAP_NAME, "temperature"}};
static const coap_endpoint_path_t path_dht_humidity = {2,  {DHT_COAP_NAME, "humidity"}};

static int handle_get_dht(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

static int handle_get_dht_temperature(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

static int handle_get_dht_humidity(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo);

const coap_endpoint_t endpoints[] =
{
    {COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core, "ct=40"},

    {COAP_METHOD_GET, handle_get_light, &path_light, "ct=0"},
    {COAP_METHOD_PUT, handle_put_light, &path_light, NULL},

    {COAP_METHOD_GET, handle_get_dht, &path_dht, "ct=0"},
    {COAP_METHOD_GET, handle_get_dht_temperature, &path_dht_temperature, "ct=0"},

    {COAP_METHOD_GET, handle_get_dht_humidity, &path_dht_humidity, "ct=0;obs"},


    {(coap_method_t)0, NULL, NULL, NULL}
};

#define OBS_RES_MAX_COUNT 2
unsigned int obs_res_count;
const coap_endpoint_path_t* obs_res_list[OBS_RES_MAX_COUNT];



void setup_dht_endpoint(char* dht_avaliable, float* humidity, float* temperature);


void build_rsp(void);
void build_obs_res_list(void);

#ifdef __cplusplus
}
#endif

#endif // ENDPOINTS_H