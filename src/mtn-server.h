#ifndef MTN_SERVER_H
#define MTN_SERVER_H

#include <stdbool.h>

#define USER_CFG_PATH "~/.config/mtn/"
#define USER_CFG_FILE_SERVER "server.conf"
#define USER_CFG_FILE_CLIENT "client.conf"

#define EXAMPLE_CFG_FILE_SERVER "server.conf.example"
#define EXAMPLE_CFG_FILE_CLIENT "client.conf.example"

void init_config();
void load_config(const char* config_file);

#define SERVER_PROPERTY_COUNT 4
typedef enum
{
    NAME = 0,
    SET,
    DESCR,
    PORT
} server_property_name;

struct server_property
{
    server_property_name name;
    char value[4096];
};

bool str_to_property_name(server_property_name* name, const char* str);

struct server_property* get_server_property_value(server_property_name name);

#endif
