#ifndef MTN_CONFIG_H
#define MTN_CONFIG_H

#include <stdbool.h>

#include "mtn-protocol.h"
#include "common.h"

struct server_config
{
    char name[SRV_NAME_LEN];
    char descr[SRV_DESCR_LEN];

    unsigned long set;
    unsigned int port;
};


struct server_config* get_server_config();

void load_server_config(const char* config_file);

void init_config(char* path);
int load_server_config_file(const char* config_file);
void parse_config(int conf_file_fd);

void set_property(const char* property, const char* value);

#endif
