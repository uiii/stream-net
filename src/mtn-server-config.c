#include "mtn-server-config.h"

#include "mtn-common.h"

void
init_server_config()
{
    char* config_array[][2] = {
        { "name", "MTN server" },
        { "descr", "" },
        { "port", SERVER_DEFAULT_PORT_TXT },
        { NULL, NULL }
    };

    init_config(config_array);
}

void
load_server_config(const char* config_file)
{
    init_server_config();
    
    int conf_file_fd;
    if(config_file)
    {
        conf_file_fd = load_file(config_file, O_RDONLY, EXAMPLE_SERVER_CFG_FILE);
    }
    else
    {
        conf_file_fd = load_file(USER_SERVER_CFG_FILE, O_RDONLY, EXAMPLE_SERVER_CFG_FILE);
    }

    if(conf_file_fd == -1)
    {
        warning("server config won't be loaded");
    }
    else
    {
        parse_config_file(conf_file_fd);
    }
}
