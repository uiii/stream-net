#include "mtn-server-client-list.h"

client_map*
server_client_map()
{
    static client_map map = NULL;
    return &map;
}

pthread_mutex_t*
server_client_map_mutex()
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    return &mutex;
}
