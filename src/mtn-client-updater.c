#include "mtn-client-updater.h"

#include <stdio.h>

#include <pthread.h>

void
update_control(client_server_list server_list)
{
    client_server_list not_subscribed_list = get_not_subscribed_list();

    struct server_list_node* node;
    DL_FOREACH(server_list, node)
    {
        DL_APPEND(not_subscribed_list, node);
    }

    pthread_t subscr_t;
    pthread_t update_t;

    pthread_create(&subscr_t, NULL, subscription_thread(), NULL);
    pthread_create(&update_t, NULL, update_thread(), NULL);
}

void
subscription_thread(void*)
{
    client_server_list not_subscribed_list = get_not_subscribed_list();

    // TODO
}

void
update_thread(void*)
{
    // TODO
}

client_server_list
get_subscribed_list()
{
    static client_server_list subscribed_list = NULL;
    return subscribed_list;
}

client_server_list
get_not_subscribed_list()
{
    static client_server_list not_subscribed_list = NULL;
    return not_subscribed_list;
}

void
add_to_non_subscribed(struct server_list_node* node)
{
    client_server_list list = get_not_subscribed_list();

    DL_APPEND(list, node);
}

void
add_to_subscribed(struct server_list_node* node, const char* server_reponse)
{
    client_server_list list = get_subscribed_list();

    // TODO response

    DL_APPEND(list, node);
}

/*void
send_request(struct server_list_node* node, const char* request,
        void (*response_callback)(struct server_list_node* node, const char* response),
        void (*non_response_callback)(struct server_list_node* node))*/
bool
send_request(struct server_data* data, const char* request, char* response)
{
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd == -1)
    {
        err(1, "Fatal error: cannot create a socket");
    }

    char ip_address_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &data->address.sin_addr, ip_address_str, INET_ADDRSTRLEN);

    if(sendto(socket_fd, request, MTN_REQ_MSG_LEN, 0,
        (struct sockaddr *) &(data->address), sizeof(data->address)) == -1)
    {
        perror("Error: cannot send %s request to %s:%d",
                request, ip_address_str, ntohs(data->port));

        /*if(non_response_callback)
        {
            non_response_callback(data);
        }*/

        return false;
    }

    printf("Sending %s request to %s:%d", ip_address_str, ntohs(data->port));

    fd_set socket_set;
    FD_ZERO(&socket_set);
    FD_SET(socket_fd, &socket_set);

    struct timeval timeout;
    timeout.tv_set = SERVER_RESPONSE_TIMEOUT;
    timeout.tv_usec = 0;

    if((select(socket_fd + 1, socket_set, NULL, NULL, &timeout)) == -1)
    {
        err(1, "Fatal error: select()");
    }

    if(FD_ISSET(socket_fd, &socket_set))
    {
        int res_len;
        char response[MTN_RES_MSG_LEN + 1];
        if(res_len = recvfrom(socket_fd, response, MTN_RES_MSG_LEN, 0, NULL, NULL))
        {
            char response_type_str[MTN_RES_TYPE_LEN];
            get_response_type_str(response_type_str, response);

            printf("Receiving %s response from %s:%d",
                    response_type_str, ip_address_str, ntohs(data->port));

            data->is_responding = true;
            data->connect_attempts_count = 0;

            response[res_len] = '\0';

            /*if(response_callback)
            {
                response_callback(data, response);
            }*/
        }
        else
        {
            perror("Error: cannot receive any response to %s request from %s:%d",
                    request, ip_address_str, ntohs(data->port));

            return false;
        }
    }
    else
    {
        data->is_responding = false;
        data->connect_attempts_count++;

        printf("Server %s:%d is not responding to %s request\n",
                ip_address_str, ntohs(data->port), request);

        /*if(non_response_callback)
        {
            non_response_callback(data);
        }*/

        return false;
    }

    return true;
}
