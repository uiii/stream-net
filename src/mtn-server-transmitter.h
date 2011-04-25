#ifndef MTN_SERVER_TRANSMITTER_H
#define MTN_SERVER_TRANSMITTER_H

#include "mtn-server-client-list.h"

void transmission_control();
void* transmission_thread(void* arg);
void* send_data_message_to_client(void* arg);

#endif
