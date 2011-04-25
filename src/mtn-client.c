
/*
 * MTN client
 *
 * (c) uiii.dev@gmail.com
 */

#include <sys/socket.h>
#include <getopt.h>
#include <pthread.h>

#include "mtn-common.h"
#include "mtn-client.h"
#include "mtn-client-server-list.h"

#include "mtn-client-receiver.h"
#include "mtn-client-updater.h"

int
get_client_socket_fd()
{
    static int socket_fd = -1;

    if(socket_fd == -1)
    {
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_fd == -1)
        {
            fatal_error_errno(1, "Fatal error: cannot create a socket");            
        }
    }

    return socket_fd;
}

int
main(int argc, char** argv)
{
    char* config_file = NULL;
    char* server_list_file = NULL;

    // get options
    struct option longopts[] = {
        { "config", required_argument, NULL, 'c' },
        { "server-list", required_argument, NULL, 'c' },
        { NULL, 0, NULL, 0},
    };

    int opt;
    while((opt = getopt_long(argc, argv, "c:s:", longopts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'c':
                config_file = optarg;
                break;
            case 's':
                server_list_file = optarg;
                break;
            case '?':
                info("usage: %s \t[-c|--config <config_file>]\n\t[-s|--server-list <server_list_file>]", argv[0]);
                exit(1);
                break;
            default:
                break;
        }
    }

    load_client_config(config_file);
    load_client_server_list(server_list_file);

    print_hr();

    receive_control();
    update_control();

    pthread_exit(NULL);
}
