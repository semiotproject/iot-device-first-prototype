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

typedef struct
{
    char* hostName;
    unsigned int hostNameLenght;
    long unsigned int port;
    coap_buffer_t tok;          /* Token value */
    uint8_t tkl;                /* Token length: indicates length of the Token field */
} coap_observer_t;

static unsigned long int observersCount=0;
coap_observer_t observers[MAX_OBSERVERS_COUNT];

int addCoAPObserver(char* hostName,unsigned int hostNameLenght, long unsigned int port, coap_buffer_t tok, uint8_t tkl);
int removeCoApObserver(unsigned int observerIndex);    
    
#ifdef __cplusplus
}
#endif

#endif // OBSERVERS_H