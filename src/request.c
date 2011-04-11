#include "request.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "mtn-server.h"

void
get_server_info_message(char* info_message)
{
    struct server_config* server = get_server_config();

    char message[MTN_RES_MSG_LEN + 1];

    size_t position = 0;
    strcopy(message, &position, "\nNAME: ", -1);
    strcopy(message, &position, server->name, SRV_NAME_LEN);
    strcopy(message, &position, "\nSET: ", -1);
    
    char set[SRV_SET_LEN];
    sprintf(set, "%ld", server->set);
    strcopy(message, &position, set, SRV_SET_LEN);

    strcopy(message, &position, "\nDESCR: ", -1);
    strcopy(message, &position, server->descr, SRV_DESCR_LEN);

    message[position] = '\0';

    strcpy(info_message, message);
}

void
request_control(int socket_fd)
{
    char request_msg[MTN_REQ_MSG_LEN + 1];
    char response_msg[MTN_RES_MSG_LEN + 1];
    ssize_t req_len;
    //struct sn_request req;

    struct sockaddr_in address;
    socklen_t addr_len;

        printf("listen\n");
    while((req_len = recvfrom(socket_fd, request_msg, MTN_REQ_MSG_LEN, 0,
                    (struct sockaddr*) &address, &addr_len)) > 0)
    {
        printf("recieve\n");
        mtn_request_type req_type = get_request_type(request_msg, req_len);
        switch(req_type)
        {
            case MTN_REQ_SUBSCR: // request for subscription
                printf("subscr\n");
                get_server_info_message(response_msg);
                printf("%s\n", response_msg);
                sendto(socket_fd, response_msg, MTN_RES_MSG_LEN, 0,
                        (struct sockaddr*) &address, addr_len);
                break;
            case MTN_REQ_UPDATE: // subscription update
            case MTN_REQ_RESEND: // request for packet resending
                break;
            case MTN_REQ_UNKNOWN: // unknown request
                break;
        }
    }

    exit(0);
}
