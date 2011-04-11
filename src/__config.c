#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "common.h"

void
init_config(char* path)
{
    str_replace("~", getenv("HOME"), USER_CFG_PATH, path);
    if(access(path, F_OK) != 0) // config directory does not exists
    {
        if(mkdir(path, 0755) == -1)
        {
            err(1, "Cannot create config directory");
        }
    }

    char file_path[4096];

    strcpy(file_path, path);
    strcat(file_path, USER_CFG_FILE_SERVER);
    if(access(file_path, F_OK) != 0) // server config file does not exists
    {
        bool error = false;

        int inf, outf;
        // copy example config file 
        if((inf = open(EXAMPLE_CFG_FILE_SERVER, O_RDONLY)) != -1)
        {
            if((outf = creat(file_path, 0655)) != -1)
            {
                char buffer[4096];
                ssize_t read_len, write_len;
                while((read_len = read(inf, buffer, 4096)) > 0)
                {
                    write_len = write(outf, buffer, read_len);
                    if(write_len != read_len)
                    {
                        error = true;
                    }
                }

                close(outf);
            }
            else { error = true; }

            close(inf);
        }
        else { error = true; }

        if(error)
        {
            err(1, "Cannot create server config file");
        }
    }
}

int
load_config_file(const char* config_file)
{
    int conf_file_fd;
    if(config_file == NULL)
    {
        char file_path[4096];
        init_config(file_path);

        strcat(file_path, USER_CFG_FILE_SERVER);

        conf_file_fd = open(file_path, O_RDONLY);
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
    extern struct server_config server;

    char token[CONFIG_TOKEN_MAX_SIZE + 1];
    int token_len;
    char* token_ptr;

    char property[CONFIG_TOKEN_MAX_SIZE + 1];
    bool get_property = true;

    char value[CONFIG_TOKEN_MAX_SIZE + 1] = "";
    bool get_value = false;
    bool is_string_value = false;

    bool next_line = false;
    char line[CONFIG_LINE_MAX_SIZE + 1];
    while(get_config_line(line, conf_file_fd, true) != -1)
    {
        //printf("----> %s\n", line);
        bool from_beginning = true;
        while((token_len = get_config_token(token, line, from_beginning)))
        {
            from_beginning = false;

            if(get_property)
            {
                strcpy(property, token);

                if(property[0] == '#')
                {
                    // commnet -> go to next line
                    next_line = true;
                }

                get_property = false;
                get_value = true;
            }
            else if(get_value)
            {
                if(strcmp(value, "")) // value is nonempty
                {
                    strcat(value, " ");
                }

                if((token[0] == '"'))
                {
                    is_string_value = true;
                    token_ptr = token + 1;
                }
                else
                {
                    token_ptr = token;
                }

                if(is_string_value && (token[token_len - 1] == '"'))
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

                if(strcmp(property, "NAME") == 0)
                {
                    strcpy(server.name, value);
                }
                else if(strcmp(property, "SET") == 0)
                {
                    long number = strtoul(value, NULL, 10);
                    if(number == ULONG_MAX)
                    {
                        err(1, "Bad SET value");
                    }
                    else
                    {
                        strcpy(server.set, number);
                    }
                }
                else if(strcmp(property, "DESCR") == 0)
                {
                    strcpy(server.descr, value);
                }
                else if(strcmp(property, "PORT") == 0)
                {
                    long number = strtoul(value, NULL, 10);
                    if(number == ULONG_MAX)
                    {
                        err(1, "Bad PORT value");
                    }
                    else
                    {
                        strcpy(server.port, port);
                    }
                }
                else
                {
                    fprintf(stderr, "Error: Bad property %s\n", property);
                }

                next_line = true;
            }

            if(next_line)
            {
                strcpy(property, "");
                strcpy(value, "");

                next_line = false;
                get_property = true;
                get_value = false;
                
                break;
            }

            /*if(is_string_token == false)
            {
                // process token

                // comment -> go to next line
                if(token[0] == '#') break;

                printf("\t[%s]\n", token);

                if(strcmp(token, "NAME") == 0)
                {
                    server.name = 
                }
            }*/
        }
    }

    printf("--- eof ---\n");
}

int
get_config_line(char* line, int conf_file_fd, bool nonempty)
{
    static char buffer[CONFIG_LINE_MAX_SIZE + 1] = "";
    static char* buffer_ptr = buffer;
    static int read_len = 0;

    if(read_len == 0)
    {
        read_len = read(conf_file_fd, buffer, CONFIG_LINE_MAX_SIZE);
        buffer[read_len] = '\0';
        buffer_ptr = buffer;
    }

    if(read_len)
    {
        if(nonempty)
        {
            int skip_len = strspn(buffer_ptr, "\n");
            buffer_ptr += skip_len;
            read_len -= skip_len;
        }

        int line_len = strcspn(buffer_ptr, "\n");

        strncpy(line, buffer_ptr, line_len);
        line[line_len] = '\0';

        buffer_ptr += line_len + 1;
        read_len -= line_len + 1;

        if(read_len < 0)
        {
            read_len = 0;
            
            char next_line[CONFIG_LINE_MAX_SIZE + 1];
            int next_line_len = get_config_line(next_line, conf_file_fd, false);
            
            if(next_line_len)
            {
                line_len += next_line_len;
                if(line_len > CONFIG_LINE_MAX_SIZE)
                {
                    fprintf(stderr, "Error: Line in config file is too long");
                    exit(1);
                }
                else
                {
                    strcat(line, next_line);
                }
            }
        }

        return line_len;
    }
    else
    {
        strcpy(line, "");
        return -1;
    }
}

int
get_config_token(char* token, const char* line, bool from_beginning)
{
    static const char* line_ptr = 0;

    if(from_beginning)
    {
        line_ptr = line;
    }

    int skip_len = strspn(line_ptr, " \t");
    line_ptr += skip_len;

    int token_len = strcspn(line_ptr, " \t");
    strncpy(token, line_ptr, token_len);
    token[token_len] = '\0';

    line_ptr += token_len;

    return token_len;
}

// TODO

    /*int read_len;
    char buffer[4096];
    char tmp[8192];
    char* line = NULL;
    char* next_line = NULL;
    char* lineptr;
    char* propertyptr;
    while((read_len = read(inf, buffer, 4096)) > 0)
    {
        bool parse_last_line = false;
        if(buffer[read_len - 1] == '\n')
        {
            parse_last_line = true;
        }

        // split buffer into lines
        next_line = strtok_r(buffer, "\n", &lineptr);

        if(line != NULL)
        {
            strcpy(tmp, line);
            strcat(tmp, next_line);
            line = tmp;
        }
        else
        {
            line = next_line;
        }

        while(line != NULL)
        {
            next_line = strtok_r(NULL, "\n", &lineptr);
            if(next_line == NULL && parse_last_line == false)
            {
                break;       
            }
            else
            {
                printf("%s\n", line);
                printf("%s\n", next_line);
                char* property_str;
                server_property_name property_name;
                struct server_property* property;
                char* value;

                while((property_str = strtok_r(line, " \t", &propertyptr)) != NULL)
                {
                    line = NULL;

                    if(property_str[0] == '#') // comment
                    {
                        printf("comment\n");
                        break;
                    }

                    if(str_to_property_name(&property_name, property_str))
                    {
                        property = get_server_property_value(property_name);
                    }
                    else
                    {
                        close(inf);
                        err(1, "Bad property in sever config file"); // TODO err ne - neni treba vypisovat errno
                    }

                    value = strtok_r(NULL, " \t", &propertyptr);
                    if(value != NULL)
                    {
                        strcpy(property->value, value);
                    }
                    else
                    {
                        close(inf);
                        err(1, "Bad syntax in server config file: missing value");
                    }
                }

                line = next_line;
            }
        }
    }
}

bool
str_to_property_name(server_property_name* name, const char* str)
{
    if(strcmp(str, "NAME")) { *name = NAME; }
    else if(strcmp(str, "SET")) { *name = SET; }
    else if(strcmp(str, "DESCR")) { *name = DESCR; }
    else if(strcmp(str, "PORT")) { *name = PORT; }
    else
    {
        return false;
    }

    return true;
}

struct server_property*
get_server_property_value(server_property_name name)
{
    static struct server_property server_config[] = {
        {NAME, "MTN server"},
        {SET, "60"},
        {DESCR, ""},
        {PORT, "48888"}
    };

    return &server_config[name];
}
*/
