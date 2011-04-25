#ifndef MTN_CLIENT_H
#define MTN_CLIENT_H

#include "mtn-client-config.h"

#define SERVER_RESPONSE_TIMEOUT 3 // TODO

#define MAX_NOT_CONNECT_COUNT 3
#define NEXT_CONNECT_ATTEMPT_TIME 30 // TODO

#define MAX_ACTIVE_REQUESTS 3

int get_client_socket_fd();

#endif
