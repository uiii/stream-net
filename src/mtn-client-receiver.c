#include "mtn-client-receiver.h"

#include <pthread.h>
#include <netinet/in.h>

#include "mtn-client.h"
#include "mtn-client-request.h"
#include "mtn-client-data-message.h"

void receive_control()
{
    pthread_t recv_t;
    pthread_create(&recv_t, NULL, receive_thread, NULL);
}

void*
receive_thread(void* arg)
{
    int socket_fd = get_client_socket_fd();

    struct sockaddr_in server_address;
    socklen_t server_address_len = sizeof(server_address);

    ssize_t msg_len;
    char msg[MTN_MSG_LEN + 1];
    while((msg_len = recvfrom(socket_fd, msg, MTN_MSG_LEN, 0,
                (struct sockaddr*) &server_address, &server_address_len)))
    {
        msg[msg_len] = '\0';

        if(msg_len >= MTN_MSG_TYPE_LEN)
        {
            mtn_message_type type = get_message_type(msg);

            switch(type)
            {
                case MTN_RESPONSE:
                    receive_response(msg, &server_address);
                    break;
                case MTN_DM_HEAD:
                    receive_data_message_head(msg, &server_address);
                    break;
                case MTN_DM_BLOCK:
                    receive_data_message_block(msg, &server_address);
                    break;
                default:
                    break;
            }
        }
    }

    pthread_exit(NULL);
}

