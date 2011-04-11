
/*
 * MTN client
 *
 * (c) uiii.dev@gmail.com
 */

#include "mtn-server.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <getopt.h>
#include <stdbool.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "common.h"

#include "request.h"
#include "transmission.h"

int
main(int argc, char** argv)
{
    char* server_list_file = NULL;

    // get options
    struct option longopts[] = {
        { "server-list", required_argument, NULL, 'c' },
        { NULL, 0, NULL, 0},
    };

    int opt;
    while((opt = getopt_long(argc, argv, "s:", longopts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'c':
                server_list_file = optarg;
            break;
            case '?':
               fprintf(stderr, "usage: %s [-s|--server-list <server_list_file>]\n", argv[0]);
               exit(1);
            break;
        }
    }

    client_server_list list = load_server_list(server_list_file);

    update_control(list);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, updating_thread);
    send_subscriptions();
}
