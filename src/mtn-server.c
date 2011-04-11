/*
 * MTN server
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

#include "common.h"

#include "request.h"
#include "transmission.h"

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
    load_server_config(config_file);

    // bind socket
    int port = get_server_config()->port;

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
