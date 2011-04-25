/*
 * MTN server
 *
 * (c) uiii.dev@gmail.com
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "mtn-common.h"
#include "mtn-server.h"

#include "mtn-server-receiver.h"
#include "mtn-server-transmitter.h"
#include "mtn-server-data-message.h"

int
server_socket_fd()
{
    static int socket_fd = -1;

    if(socket_fd == -1)
    {
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(socket_fd == -1)
        {
            fatal_error_errno(1, "Fatal error: cannot create a socket");            
        }

        int port = atoi(get_config_value("port"));

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(socket_fd, (struct sockaddr*) &address, sizeof(address)) == -1)
        {
            fatal_error_errno(1, "Cannot bind socket with address");
        }
    }

    return socket_fd;
}

void
create_pid_file()
{
    pid_t pid = getpid();

    int file_fd = load_file(SERVER_PID_FILE, O_WRONLY | O_TRUNC, NULL);
    if(file_fd != -1)
    {
        char pid_str[10];
        int pid_len = snprintf(pid_str, 10, "%d", pid);

        if(write(file_fd, pid_str, pid_len) == -1)
        {
            fatal_error_errno(1, "cannot write PID to the PID file %s", SERVER_PID_FILE);
        }

        info("writing server PID %d to PID file %s", pid, SERVER_PID_FILE);

        close(file_fd);
    }
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
               info("usage: %s [-c|--config <config_file>]\n", argv[0]);
               exit(1);
            break;
        }
    }

    // handle config
    load_server_config(config_file);

    print_hr();

    create_pid_file();

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = new_message_signal;
    act.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &act, NULL);    

    request_control();
    transmission_control();
    data_message_control();

    pthread_exit(NULL);
}
