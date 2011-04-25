#include "mtn-client-data-message.h"

void
receive_data_message_head(char* msg, struct sockaddr_in* server_address)
{
    struct server_map_node* node;
    HASH_FIND(hh, *client_server_map(), server_address, sizeof(server_map_key), node);

    if(node)
    {
        struct server_data* server = node->data;

        char* msg_ptr = msg + MSG_TYPE_LEN + 1;

        char token[4096];
        get_token(token, &msg_ptr, " ");
        
        // TODO
    }
}

void
receive_data_message_block(char* msg, struct sockaddr_in* server_address)
{
    struct server_map_node* node;
    HASH_FIND(hh, *client_server_map(), server_address, sizeof(server_map_key), node);

    if(node)
    {
        struct server_data* server = node->data;

        // TODO
    }
}
