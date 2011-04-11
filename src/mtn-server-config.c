#include "mtn-server-config.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>

struct server_config* get_server_config()
{
    static struct server_config server = { "MTN server", "", SERVER_DEFAULT_SET, SERVER_DEFAULT_PORT };
    return &server;
}

void
load_server_config(const char* config_file)
{
    int conf_file_fd;
    conf_file_fd = load_server_config_file(config_file);

    parse_config(conf_file_fd);
}

int
load_server_config_file(const char* config_file)
{
    int conf_file_fd;
    if(config_file == NULL)
    {
        char path[4096];

        str_replace("~", getenv("HOME"), USER_CFG_PATH, path);
        init_dir(path);

        strcat(path, USER_SERVER_CFG_FILE);
        init_file(path, EXAMPLE_SERVER_CFG_FILE);

        conf_file_fd = open(path, O_RDONLY);
    }
    else
    {
        conf_file_fd = open(config_file, O_RDONLY);
    }

    if(conf_file_fd == -1)
    {
        err(1, "Cannot open server config file");
    }

    return conf_file_fd;
}

void parse_config(int conf_file_fd)
{
    char token[TOKEN_MAX_SIZE + 1];
    int token_len;
    char* token_ptr;

    char property[TOKEN_MAX_SIZE + 1];
    bool get_property = true;

    char value[TOKEN_MAX_SIZE + 1] = "";
    bool get_value = false;
    bool is_string_value = false;

    bool next_line = false;
    char line[LINE_MAX_SIZE + 1];
    while(get_file_line(line, conf_file_fd, true) != -1)
    {
        if(get_value && ! is_string_value)
        {
            fprintf(stderr, "Error: Missing value of %s\n", property);

            get_value = false;
            get_property = true;
        }

        //printf("----> %s\n", line);
        bool from_beginning = true;
        while((token_len = get_token(token, line, " \t", from_beginning)))
        {
            from_beginning = false;

            if(get_property)
            {
                strcpy(property, token);

                if(property[0] == '#')
                {
                    // commnet -> go to next line
                    next_line = true;
                    break;
                }

                get_property = false;
                get_value = true;
            }
            else if(get_value)
            {
                token_ptr = token;

                if(strcmp(value, "")) // value is nonempty
                {
                    strcat(value, " ");
                }

                if((token_ptr[0] == '"'))
                {
                    is_string_value = true;
                    token_ptr += 1;
                    token_len -= 1;
                }
                else
                {
                    token_ptr = token;
                }

                if(is_string_value && (token_ptr[token_len - 1] == '"'))
                {
                    strncat(value, token_ptr, token_len - 1);

                    is_string_value = false;
                    get_value = false;
                }
                else
                {
                    strcat(value, token_ptr);

                    if(! is_string_value)
                    {
                        get_value = false;
                    }
                }
            }

            if((get_property == false) && (get_value == false))
            {
                printf("%s = %s\n", property, value);

                set_property(property, value);

                next_line = true;
            }
        }

        if(next_line)
        {
            strcpy(property, "");
            strcpy(value, "");

            next_line = false;
            get_property = true;
            get_value = false;
        }
    }

    printf("--- eof ---\n");

    struct server_config* server = get_server_config();

    printf("%s\n", server->name);
    printf("%ld\n", server->set);
    printf("%s\n", server->descr);
    printf("%d\n", server->port);
}

void
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
}
