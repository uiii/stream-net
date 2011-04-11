#ifndef MTN_CLIENT_UPDATER_H
#define MTN_CLIENT_UPDATER_H

#include "mtn-client-server-list.h"

void update_control(client_server_list list);

void send_request(struct server_data* data, const char* request, void (*response_callback)(const char* response));

#endif
