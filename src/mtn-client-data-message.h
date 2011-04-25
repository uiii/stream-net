#ifndef MTN_CLIENT_DATA_MESSAGE_H
#define MTN_CLIENT_DATA_MESSAGE_H

#include <netinet/in.h>

#include "uthash.h"

#include "mtn-protocol.h"

struct data_message_block
{
    char data[MTN_DM_BLOCK_DATA_LEN + 1];

    struct data_message_block* next;
    struct data_message_block* prev;
};

typedef struct data_message_block* block_list;

struct data_message
{
    time_t release_time;
    int id;
    
    time_t receive_time;

    int blocks_count;
    block_list blocks;
};

struct message_map_key
{
    time_t time;
    int id;
};

struct message_map_node
{
    struct data_message* data;
    struct message_map_key key;

    UT_hash_handle hh;
};

typedef struct message_map_node* message_map;

void receive_data_message_head(char* msg, struct sockaddr_in* server_address);
void receive_data_message_block(char* msg, struct sockaddr_in* server_address);

#endif
