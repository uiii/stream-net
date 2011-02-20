/*
 * Stream-net transmitter
 *
 * (c) uiii.dev@gmail.com
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "config.h"

#include "subscription.h"
#include "transmission.h"

int
main(int argc, char** argv)
{
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd == -1 || 1)
    {
        err(1, "Cannot create a socket");
    }

    int pid = 0;
    switch(pid = fork())
    {
        case -1:
            // error
            exit(1);
        case 0:
            //subscription_control(socket_fd);
            break;
        default:
            //transmission_control(socket_fd);
            break;
    }

    return 0;
}
