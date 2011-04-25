#ifndef MTN_CLIENT_UPDATER_H
#define MTN_CLIENT_UPDATER_H

#include "mtn-client-server-list.h"

void update_control();

void* send_subscription_request_thread(void* server_data);

void* subscription_thread(void*);
void* update_thread(void*);

#endif
