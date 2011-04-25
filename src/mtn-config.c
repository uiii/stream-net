#include "mtn-config.h"

#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#include "mtn-common.h"

config_map*
get_config()
{
    static config_map config = NULL;
    return &config;
}

#define COMMON_CONFIG_COUNT 1

void
init_config(char* (*config_array)[2])
{
    char* common_config_array[][2] = {
        { "debug", "0" },
        { NULL, NULL }
    };

    init_config_from_array(common_config_array);

    if(config_array)
    {
        init_config_from_array(config_array);
    }
}

void
init_config_from_array(char* (*config_array)[2])
{
    config_map* config = get_config();

    struct config_node* node;
    for(char* (*config_item)[2] = config_array;
            (*config_item)[0] != NULL && (*config_item)[1] != NULL;
            config_item++)
    {
        node = (struct config_node*) malloc(sizeof(struct config_node));
        char* value = (char*) malloc(sizeof(value));
        if((node == NULL) || (value == NULL))
        {
            fatal_error(1, "cannot allocate memory");
        }

        info("initialize config '%s' to '%s'", (*config_item)[0], (*config_item)[1]);

        strcpy(node->name, *config_item[0]);

        node->value = value;
        strcpy(node->value, (*config_item)[1]);

        HASH_ADD_STR(*config, name, node);
    }
}

char*
get_config_value(const char* name)
{
    config_map* config = get_config();

    struct config_node* node;
    HASH_FIND_STR(*config, name, node);

    if(node)
    {
        return node->value;
    }
    else
    {
        warning("cannot get config '%s': unknown config name '%s'", name, name);
        return NULL;
    }
}

bool
set_config_value(const char* name, const char* value)
{
    config_map* config = get_config();

    struct config_node* node;
    HASH_FIND_STR(*config, name, node);

    if(node)
    {
        char* new_value = realloc(node->value, sizeof(value));
        if(new_value == NULL)
        {
            fatal_error(1, "cannot reallocate memory");
        }

        node->value = new_value;
        strcpy(node->value, value);
    }
    else
    {
        warning("cannot set config '%s': unknown config name '%s'", name, name);
        return false;

        /*node = (struct config_node*) malloc(sizeof(struct config_node));
        char* new_value = (char*) malloc(sizeof(value));
        if((node == NULL) || (new_value == NULL))
        {
            fatal_error(1, "cannot allocate memory");
        }

        strcpy(node->name, name);

        node->value = new_value;
        strcpy(node->value, value);

        HASH_ADD_STR(*config, name, node);*/
    }

    return true;
}

void parse_config_file(int conf_file_fd)
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
            error("missing value of %s\n", property);

            get_value = false;
            get_property = true;
        }

        //printf("----> %s\n", line);
        char* line_ptr = line;

        while((token_len = get_token(token, &line_ptr, " \t")))
        {
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
                if(set_config_value(property, value))
                {
                    info("load and set config '%s' to '%s'", property, value);
                }

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
}
