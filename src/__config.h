#ifndef MTN_CONFIG_H
#define MTN_CONFIG_H

#include <stdbool.h>

#define CONFIG_LINE_MAX_SIZE 4096
#define CONFIG_TOKEN_MAX_SIZE 4096

#define USER_CFG_PATH "~/.config/mtn/"

#define USER_SERVER_CFG_FILE "server.conf"

#define USER_CLIENT_CFG_FILE "client.conf"
#define USER_CLIENT_SERVER_LIST_FILE "client/server_list"

#define EXAMPLE_SERVER_CFG_FILE "server.conf.example"
#define EXAMPLE_CLIENT_CFG_FILE "client.conf.example"

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

void init_config(char* path);
int load_config_file(const char* config_file);
void parse_config(int conf_file_fd);

int get_config_line(char* line, int conf_file_fd, bool nonempty);
int get_config_token(char* token, const char* line, bool from_beginning);

/*bool str_to_property_name(server_property_name* name, const char* str);
struct server_property* get_server_property_value(server_property_name name);*/

#endif
