#include "mtn-client-config.h"

#include "mtn-common.h"

void
init_client_config()
{
    init_config(NULL);
}

void
load_client_config(const char* config_file)
{
    init_client_config();

    int conf_file_fd;
    if(config_file)
    {
        conf_file_fd = load_file(config_file, O_RDONLY, EXAMPLE_CLIENT_CFG_FILE);
    }
    else
    {
        conf_file_fd = load_file(USER_CLIENT_CFG_FILE, O_RDONLY, EXAMPLE_CLIENT_CFG_FILE);
    }

    if(conf_file_fd == -1)
    {
        warning("client config won't be loaded");
    }
    else
    {
        parse_config_file(conf_file_fd);
    }
}

/*void
set_property(const char* property, const char* value)
{
    struct server_config* server = get_server_config();

    if(strcmp(property, "NAME") == 0)
    {
        if(strlen(value) > SRV_NAME_LEN)
        {
            fprintf(stderr, "Error: maximum length of server name is %d\n", SRV_NAME_LEN);
        }
        else
        {
            strcpy(server->name, value);
        }
    }
    else if(strcmp(property, "SET") == 0)
    {
        long number = strtoul(value, NULL, 10);
        if(number > 0 && number < 100000000)
        {
            server->set = number;
        }
        else
        {
            fprintf(stderr, "Error: SET value has to be in 0 - 1 000 000 000 range\n");
        }
    }
    else if(strcmp(property, "DESCR") == 0)
    {
        if(strlen(value) > SRV_DESCR_LEN)
        {
            fprintf(stderr, "Error: maximum length of server description is %d\n", SRV_DESCR_LEN);
        }
        else
        {
            strcpy(server->descr, value);
        }
    }
    else if(strcmp(property, "PORT") == 0)
    {
        long number = strtoul(value, NULL, 10);
        if(number >= 1024 && number < 65535)
        {
            server->port = number;
        }
        else
        {
            fprintf(stderr, "Error: port number has to be in 1024 - 65535 range\n");
        }
    }
    else
    {
        fprintf(stderr, "Error: Bad property %s\n", property);
    }
}*/
