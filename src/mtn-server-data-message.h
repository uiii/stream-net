#ifndef MTN_SERVER_DATA_MESSAGE_H
#define MTN_SERVER_DATA_MESSAGE_H

#include <time.h>
#include <dirent.h>
#include <pthread.h>

#include "uthash.h"

#include "mtn-protocol.h"
#include "mtn-config.h"

#define USER_NEW_MESSAGE_DIR USER_CFG_PATH"server/new_messages/"
#define USER_SENT_MESSAGE_DIR USER_CFG_PATH"server/sent_messages/"

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
    int id; // unique id for the release_time

    char file_name[NAME_MAX + 1];

    int blocks_count;
    block_list blocks;
};

struct message_queue_node
{
    struct data_message* data;

    struct message_queue_node* next;
    struct message_queue_node* prev;
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

typedef struct message_queue_node* message_queue;
typedef struct message_map_node* message_map;

message_queue* to_send_queue();
pthread_mutex_t* to_send_queue_mutex();
pthread_cond_t* to_send_queue_cond();

message_map* sent_map();
pthread_mutex_t* sent_map_mutex();
pthread_cond_t* sent_map_cond();

void new_message_signal(int arg);

void data_message_control();
void* load_data_message_thread(void* arg);

#endif
