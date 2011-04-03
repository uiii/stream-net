/*
 * MTN transmitter
 *
 * (c) uiii.dev@gmail.com
 */

#include "mtn-server.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>

#include <sys/wait.h>
#include <sys/stat.h>

#include "config.h"
#include "common.h"

#include "request.h"
#include "transmission.h"

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

void
load_config(const char* config_file)
{
    int inf;
    if(config_file == NULL)
    {
        char file_path[4096];
        init_config(file_path);

        strcat(file_path, USER_CFG_FILE_SERVER);

        inf = open(file_path, O_RDONLY);
    }
    else
    {
        inf = open(config_file, O_RDONLY);
    }

    if(inf == -1)
    {
        err(1, "Cannot open server config file");
    }

    int read_len;
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

                    if(property_str[0] = '#') // comment
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

                    value = strtok_r(NULL, " \t", property_ptr);
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
    if(strcmp(str, "NAME")) { name = NAME; }
    else if(strcmp(str, "SET")) { name = SET; }
    else if(strcmp(str, "DESCR")) { name = DESCR; }
    else if(strcmp(str, "PORT")) { name = PORT; }
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

int
main(int argc, char** argv)
{
    char* config_file = NULL;

    // get options
    struct option longopts[] = {
        { "config", required_argument, NULL, 'c' },
        { NULL, 0, NULL, 0},
    };

    int opt;
    while((opt = getopt_long(argc, argv, "c:", longopts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'c':
                config_file = optarg;
            break;
            case '?':
               fprintf(stderr, "usage: %s [-c|--config <config_file>]\n", argv[0]);
               exit(1);
            break;
        }
    }

    // handle config
    load_config(config_file);

    // bind socket
    int port = SERVER_PORT;

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd == -1)
    {
        err(1, "Cannot create a socket");
    }

    struct sockaddr_in in;
    memset(&in, 0, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(port);
    in.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socket_fd, (struct sockaddr*) &in, sizeof(in)) == -1)
    {
        err(1, "Cannot bind socket with address");
    }

    int pid = 0;
    switch(pid = fork())
    {
        case -1:
            err(1, "Cannot start MTN server");
        case 0:
            request_control(socket_fd);
            break;
        default:
            transmission_control(socket_fd);
            break;
    }

    waitpid(pid, NULL, 0);

    close(socket_fd);

    return 0;
}
