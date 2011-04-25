#include "mtn-server-receiver.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "mtn-common.h"
#include "mtn-server.h"

/*void
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
}*/

void
request_control()
{
    pthread_t rc_t;
    pthread_create(&rc_t, NULL, request_control_thread, NULL);
}

void*
request_control_thread(void* arg)
{
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    ssize_t msg_len;
    char msg[MTN_MSG_LEN + 1];
    while((msg_len = recvfrom(server_socket_fd(), msg, MTN_MSG_LEN, 0,
                (struct sockaddr*) &client_address, &client_address_len)))
    {
        msg[msg_len] = '\0';

        if(msg_len >= MTN_MSG_TYPE_LEN)
        {
            mtn_message_type type = get_message_type(msg);

            mtn_request_type req_type;
            switch(type)
            {
                case MTN_REQUEST:
                    req_type = get_request_type(msg);

                    switch(req_type)
                    {
                        case MTN_REQ_SUBSCR:
                            receive_subscription(msg, client_address);
                            break;
                        case MTN_REQ_RESEND:
                            receive_resend(msg, client_address);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    pthread_exit(NULL);
}

void
receive_subscription(const char* msg, struct sockaddr_in client_address)
{
    char ip_address_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, ip_address_str, INET_ADDRSTRLEN);

    int port = ntohs(client_address.sin_port);    

    info("receiving subscription request from %s:%d", ip_address_str, port);

    struct client_map_node* node;
    HASH_FIND(hh, *server_client_map(), &client_address, sizeof(client_address), node);

    if(node)
    {
        pthread_mutex_lock(&node->data->mutex);

        node->data->last_update_time = current_time();
        
        pthread_mutex_unlock(&node->data->mutex);

        info("client %s:%d is already subscribed ... ACCEPTING request", ip_address_str, port);
    }
    else
    {
        node = (struct client_map_node*) malloc(sizeof(struct client_map_node));
        struct client_data* client = (struct client_data*) malloc(sizeof(struct client_data));
        if(node == NULL || client == NULL)
        {
            fatal_error_errno(1, "cannot allocate memory");
        }

        memset(node, 0, sizeof(node));
        memset(client, 0, sizeof(client));

        client->address = client_address;
        client->last_update_time = current_time();

        pthread_mutex_init(&client->mutex, NULL);

        node->key = client_address;
        node->data = client;

        HASH_ADD(hh, *server_client_map(), key, sizeof(client_map_key), node);

        info("client %s:%d is not yet subscribed ... ACCEPTING request", ip_address_str, port);
    }

    char response[MTN_MSG_LEN + 1];
    int response_length = snprintf(response, MTN_MSG_LEN + 1, "%s %s\nname %s\ndescr %s",
            MTN_RESPONSE_TXT, MTN_RES_ACCEPT_TXT,
            get_config_value("name"),
            get_config_value("descr")
    );

    send_response(client_address, response, response_length, MTN_REQ_SUBSCR);
}

void receive_resend(const char* msg, struct sockaddr_in client_address)
{
}

bool
send_response(struct sockaddr_in client_address, const char* response, int response_length, mtn_request_type request_type)
{
    char ip_address_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, ip_address_str, INET_ADDRSTRLEN);

    int port = ntohs(client_address.sin_port);    

    char response_type_str[MTN_RES_TYPE_LEN + 1];
    get_response_type_str(response_type_str, get_response_type(response));    

    char request_type_str[MTN_REQ_TYPE_LEN + 1];
    get_request_type_str(request_type_str, request_type);    

    if(sendto(server_socket_fd(), response, response_length, 0,
        (struct sockaddr *) &(client_address), sizeof(client_address)) == -1)
    {
        error_errno("cannot send %s response to %s request to %s:%d client",
                response_type_str, request_type_str, ip_address_str, port);

        return false;
    }

    info("Sending %s response to %s request of %s:%d client", response_type_str, request_type_str, ip_address_str, port);

    return true;
}
