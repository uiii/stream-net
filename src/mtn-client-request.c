#include "mtn-client-request.h"

#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "mtn-common.h"

#include "mtn-client.h"

static pthread_cond_t response_cond = PTHREAD_COND_INITIALIZER;

bool
send_request(struct server_data* server, const char* request, int request_length)
{
    int socket_fd = get_client_socket_fd();

    char ip_address_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server->address.sin_addr, ip_address_str, INET_ADDRSTRLEN);

    int port = ntohs(server->address.sin_port);

    char request_type_str[MTN_REQ_TYPE_LEN + 1];
    get_request_type_str(request_type_str, get_request_type(request));
    
    if(sendto(socket_fd, request, request_length, 0,
        (struct sockaddr *) &(server->address), sizeof(server->address)) == -1)
    {
        error_errno("cannot send %s request to %s:%d",
                request_type_str, ip_address_str, port);

        return false;
    }

    time_t send_time;
    if((send_time = time(NULL)) == -1)
    {
        fatal_error(1, "cannot measure time");
    }

    info("Sending %s request to %s:%d", request_type_str, ip_address_str, port);

    struct timespec timeout;
    timeout.tv_sec = send_time + SERVER_RESPONSE_TIMEOUT;
    timeout.tv_nsec = 0;

    pthread_mutex_lock(&server->mutex);

    while(server->state != RESP)
    {
        if(pthread_cond_timedwait(&response_cond, &server->mutex, &timeout) == ETIMEDOUT)
        {
            info("Server %s:%d is not responding to %s request",
                    ip_address_str, port, request_type_str);

            server->last_connect_attempt_time = send_time;
            server->not_connect_count++;

            if(server->not_connect_count % MAX_NOT_CONNECT_COUNT == 0)
            {
                // client runs out of attempts to connect to the server
                // client now waits for next attempt
                server->state = WAIT;
            }

            pthread_mutex_unlock(&server->mutex);

            return false;
        }
    }

    char response_type_str[MTN_RES_TYPE_LEN + 1];
    get_response_type_str(response_type_str, get_response_type(server->response));

    time_t connect_time;
    if((connect_time = time(NULL)) == -1)
    {
        fatal_error(1, "cannot measure time");
    }

    info("Receiving %s response from %s:%d",
            response_type_str, ip_address_str, port);

    info("%s", server->response);

    server->last_connect_attempt_time = connect_time;
    server->not_connect_count = 0;

    pthread_mutex_unlock(&server->mutex);

    return true;
}

void
receive_response(const char* response, struct sockaddr_in* server_address)
{
    struct server_map_node* node;
    HASH_FIND(hh, *client_server_map(), server_address, sizeof(server_map_key), node);

    pthread_mutex_lock(&node->data->mutex);

    strcpy(node->data->response, response);
    node->data->state = RESP;

    pthread_cond_broadcast(&response_cond);

    pthread_mutex_unlock(&node->data->mutex);
}
