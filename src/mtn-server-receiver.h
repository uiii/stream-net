#ifndef MTN_SERVER_RECEIVER_H
#define MTN_SERVER_RECEIVER_H

#include <netinet/in.h>
#include <stdbool.h>

#include "mtn-protocol.h"
#include "mtn-server-client-list.h"

void request_control();
void* request_control_thread(void*);

void receive_subscription(const char* msg, struct sockaddr_in client_address);
void receive_update(const char* msg, struct sockaddr_in client_address);
void receive_resend(const char* msg, struct sockaddr_in client_address);

bool send_response(struct sockaddr_in client, const char* response, int response_length, mtn_request_type request_type);

#endif
