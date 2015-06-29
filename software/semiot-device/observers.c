#include "observers.h"

// returns observerIndex or -1
int addCoAPObserver(const char* hostName,unsigned int hostNameLenght, long unsigned int port, coap_packet_t pkt)
{
    if ( observersCount < MAX_OBSERVERS_COUNT )
    {
        // TODO: update by token
        observers[observersCount]=(coap_observer_t){hostName, hostNameLenght, port, pkt};
        observersCount+=1;
        return observersCount;
    }
    else {
        return -1;
    }
}

// returns observersCount or -1
/*
int removeCoApObserver(unsigned int observerIndex)
{
    if (observerIndex<observersCount) {
        int i, j;
        for (i=observerIndex;i<observersCount-1;i++)
        {
            for (j=observerIndex+1;j<observersCount;i++)
            {
                observers[i]=observers[j];
            }
        }
        observersCount--;
        return observersCount;
    }
    else {
        return -1;
    }
}
*/

unsigned int getObserversCount()
{
    return observersCount;
}

int removeCoApObserver(const char* hostName,unsigned int hostNameLenght, long unsigned int port, coap_packet_t pkt)
{
    //TODO:
}
