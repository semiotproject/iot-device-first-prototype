#include "observers.h"

// returns observerIndex or -1
int addCoAPObserver(const char* hostName,unsigned int hostNameLenght, long unsigned int* port, coap_packet_t pkt, const coap_endpoint_path_t *path)
{
    if ( observersCount < MAX_OBSERVERS_COUNT )
    {
        // TODO: update by token
        observers[observersCount]=(coap_observer_t){hostName, hostNameLenght, *port, pkt,*path};
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

void delete_item(struct coap_observer_t *p,int *num_items, int item)
{

    if (*num_items > 0 && item < *num_items && item > -1) {
        int last_index = *num_items - 1;
        int i;
        for (i = item; i < last_index;i++)
        {
            p[i] = p[i + 1];
        }
        *num_items -= 1;
    }
}

bool removeCoApObserver(const char* hostName, unsigned int hostNameLenght, long unsigned int* port, coap_endpoint_path_t *path) {
    int i;
    for (i=0;i<observersCount;i++) {
        if (is_coap_endpoint_path_t_eq(&observers[i].path,path)) {
            if (observers[i].port==*port) {
                if (observers[i].hostNameLenght==hostNameLenght) {
                    if (strcmp(observers[i].hostName,hostName)) {
                        delete_item(&observers,&observersCount,i);
                        return true;
                    }
                }
            }

        }
    }
    return false;

}
