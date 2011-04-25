#ifndef MTN_CLIENT_REQUEST_H
#define MTN_CLIENT_REQUEST_H

#include <stdbool.h>
#include <netinet/in.h>

#include "mtn-client-server-list.h"

bool send_request(struct server_data* server, const char* request, int request_length);
void receive_response(const char* response, struct sockaddr_in* server_address);

#endif
