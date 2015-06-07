#include "observers.h" 

// returns observerIndex or -1
int addCoAPObserver(char* hostName,unsigned int hostNameLenght, long unsigned int port, coap_buffer_t tok, uint8_t tkl)
{
    if ( observersCount < MAX_OBSERVERS_COUNT )
    {
        observers[observersCount]=(coap_observer_t){hostName, hostNameLenght, port, tok, tkl};
        observersCount++;
        return observersCount;
    }
    else {
        return -1;
    }
}

// returns observersCount or -1
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