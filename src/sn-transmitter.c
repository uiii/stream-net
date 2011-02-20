/*
 * Stream-net transmitter
 *
 * (c) uiii.dev@gmail.com
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "config.h"

#include "subscription.h"
#include "transmission.h"

int
main(int argc, char** argv)
{
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
            // error
            exit(1);
        case 0:
            subscription_control(socket_fd);
            break;
        default:
            transmission_control(socket_fd);
            break;
    }

    return 0;
}
