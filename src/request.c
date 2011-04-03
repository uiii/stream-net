#include "request.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

struct mtn_server_info
{
    char name[SRV_NAME_LEN + 1];
    char subscr_expr_time[SRV_SET_LEN + 1];
    char descr[SRV_DESCR_LEN + 1];
};

void
get_message(char* info_message, struct mtn_server_info info)
{
    char message[MTN_RES_MSG_LEN + 1];

    size_t position = 0;
    strcopy(message, &position, "NAME: ", 1);
    strcopy(message, &position, info.name, SRV_NAME_LEN);
    strcopy(message, &position, "\nSET: ", 1);
    strcopy(message, &position, info.subscr_expr_time, SRV_SET_LEN);
    strcopy(message, &position, "\nDESCR: ", 1);
    strcopy(message, &position, info.descr, SRV_DESCR_LEN);

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

    while((req_len = recvfrom(socket_fd, request_msg, MTN_REQ_MSG_LEN, 0,
                    (struct sockaddr*) &address, &addr_len)) > 0)
    {
        mtn_request_type req_type = get_request_type(request_msg, req_len);
        struct mtn_server_info s; // TODO
        switch(req_type)
        {
            case MTN_REQ_SUBSCR: // request for subscription
                get_message(response_msg, s);
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
