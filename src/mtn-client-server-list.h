#ifndef MTN_SERVER_LIST_H
#define MTN_SERVER_LIST_H

#include "utlist.h"

#include "common.h"

struct server_data
{
    struct sockaddr_in address;

    bool is_responding;
    int connect_attempts_count;

    bool is_subscribed;

    char name[SRV_NAME_LEN];
    char descr[DESCR_NAME_LEN];
    int set;
};

struct server_list_node
{
    struct server_data* data;

    struct server_list_node* prev;
    struct server_list_node* next;
};

typedef struct server_list_node* client_server_list;

client_server_list load_client_server_list(const char* server_list_file);
int load_client_server_list_file(const char* server_list_file);

client_server_list parse_client_server_list(int server_list_file_fd);

#endif
