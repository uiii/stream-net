#ifndef MTN_SERVER_H
#define MTN_SERVER_H

#include "mtn-server-config.h"

#define SERVER_PID_FILE USER_CFG_PATH"server/server.pid"

#define MAX_ACTIVE_SENDINGS 3 //TODO

int server_socket_fd();

#endif
