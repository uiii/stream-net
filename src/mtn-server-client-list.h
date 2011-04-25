#ifndef MTN_SERVER_CLIENT_LIST_H
#define MTN_SERVER_CLIENT_LIST_H

#include <netinet/in.h>
#include <pthread.h>

#include "uthash.h"

struct client_data
{
    struct sockaddr_in address;

    time_t last_update_time;

    pthread_mutex_t mutex;
};

typedef struct sockaddr_in client_map_key;

struct client_map_node
{
    struct client_data* data;

    client_map_key key;
    UT_hash_handle hh;
};

typedef struct client_map_node* client_map;

client_map* server_client_map();
pthread_mutex_t* server_client_map_mutex();

#endif
