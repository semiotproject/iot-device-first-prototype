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

// TODO: rewrite completely!!1
typedef struct
{
    char* hostName;
    unsigned int hostNameLenght;
    long unsigned int port;
    coap_packet_t pkt;
} coap_observer_t;

static unsigned long int observersCount;
coap_observer_t observers[MAX_OBSERVERS_COUNT];

int addCoAPObserver(const char* hostName,unsigned int hostNameLenght, long unsigned int port, coap_packet_t pkt);
//int removeCoApObserver(unsigned int observerIndex);   
int removeCoApObserver(const char* hostName,unsigned int hostNameLenght, long unsigned int port, coap_packet_t pkt);
unsigned int getObserversCount(); //TODO: get rid of
    
#ifdef __cplusplus
}
#endif

#endif // OBSERVERS_H