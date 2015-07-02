#ifndef   OBSERvERS_H
#define   OBSERvERS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
    
#include "microcoap.h"
    
#define MAX_OBSERVERS_COUNT 2 // FIXME

// http://tools.ietf.org/html/draft-ietf-core-observe-16

typedef struct coap_observer_t
{
    char* hostName;
    unsigned int hostNameLenght;
    long unsigned int port;
    coap_packet_t answer_draft_pkt;
    coap_endpoint_path_t path;
} coap_observer_t;

static unsigned long int observersCount;
struct coap_observer_t observers[MAX_OBSERVERS_COUNT];

int addCoAPObserver(const char* hostName,unsigned int hostNameLenght, long unsigned int* port, coap_packet_t pkt, const coap_endpoint_path_t *path);
//int removeCoApObserver(unsigned int observerIndex);
bool removeCoApObserver(const char* hostName, unsigned int hostNameLenght, unsigned long *port, coap_endpoint_path_t* path);
unsigned int getObserversCount(); //TODO: get rid of
    
#ifdef __cplusplus
}
#endif

#endif // OBSERVERS_H
