#ifndef MTN_CONFIG_H
#define MTN_CONFIG_H

#include <stdbool.h>

#include "uthash.h"

#define USER_CFG_PATH "~/.config/mtn/"

#define USER_SERVER_CFG_FILE USER_CFG_PATH"server.conf"

#define USER_CLIENT_CFG_FILE USER_CFG_PATH"client.conf"
#define USER_CLIENT_SERVER_LIST_FILE USER_CFG_PATH"client/server_list"

#define EXAMPLE_SERVER_CFG_FILE "conf/server.conf.example"
#define EXAMPLE_CLIENT_CFG_FILE "conf/client.conf.example"

#define MTN_CFG_NAME_LEN 32
#define MTN_CFG_VALUE_LEN 4096

struct config_node
{
    char* value;
    char name[MTN_CFG_NAME_LEN];

    UT_hash_handle hh;
};

typedef struct config_node* config_map;

config_map* get_config();

void init_config(char* config_array[][2]);
void init_config_from_array(char* config_array[][2]);

char* get_config_value(const char* name);
bool set_config_value(const char* name, const char* value);

int load_config_file(const char* config_file, const char* example_file);
void parse_config_file(int config_file_fd);

#endif
