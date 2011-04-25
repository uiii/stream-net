#ifndef MTN_CLIENT_SERVER_LIST_H
#define MTN_CLIENT_SERVER_LIST_H

#include <netinet/in.h>
#include <pthread.h>

#include "uthash.h"

#include "mtn-common.h"
#include "mtn-client-data-message.h"

enum server_state
{
    SEND_REQ = 0,
    RESP,
    WAIT,
    SUBSCR
};

struct server_data
{
    struct sockaddr_in address;

    enum server_state state;

    int not_connect_count;
    time_t last_connect_attempt_time;

    bool is_responding;
    char response[MTN_MSG_LEN + 1];

    char name[SRV_NAME_LEN];
    char descr[SRV_DESCR_LEN];
    int set;

    message_map data_messages;

    pthread_mutex_t mutex;
};

struct server_list_node
{
    struct server_data* data;

    struct server_list_node* prev;
    struct server_list_node* next;
};

typedef struct sockaddr_in server_map_key;

struct server_map_node
{
    struct server_data* data;

    server_map_key key;
    UT_hash_handle hh;
};

typedef struct server_list_node* client_server_list;
typedef struct server_map_node* server_map;

server_map* client_server_map();

void load_client_server_list(const char* server_list_file);

server_map parse_client_server_list(int server_list_file_fd);

#endif
