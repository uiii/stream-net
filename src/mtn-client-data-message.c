#include "mtn-client-data-message.h"

#include <time.h>

static
bool
timeout_sooner(void* a, void* b)
{
	struct data_message* msg1 = (struct data_message*) a;
	struct data_message* msg2 = (struct data_message*) b;

	return msg1->receive_time < msg2->reveive_time;
}

static UT_icd icd = { sizeof(struct data_message*), NULL, NULL, NULL };

static array_heap data_message_queue = { NULL, timeout_sooner };

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
        
		struct tm tm_time;
		time_t time;
		strptime(token, "%s", &tm_time);
		if((time = mktime(&time)) == -1)
		{
			error("cannot get data message release time ... IGNORING data message");
		}
		else
		{
        	get_token(token, &msg_ptr, " ");
			int id = atoi(token);

        	get_token(token, &msg_ptr, " ");
			int blocks_count = atoi(token);

			ALLOCATE(struct data_message, message);

			message->release_time = time;
			message->id = id;
			message->blocks_count = blocks_count;

			message->receive_time = current_time();
		}

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

void*
receive_data_messages_thread()
{
	while(true)
	{
		pthread_mutex_lock(&data_message_queue_mutex);

		while(heap_is_empty(data_message_queue))
		{
			debug(5, "wait for data message timeout");
			pthread_cond_wait(&data_message_queue_cond, &data_message_queue_mutex);
		}

		bool wait = true;
		struct timespec timeout;
		struct data_message* message = *(struct data_message**) heap_first(data_message_queue);

		//while(wait && message->state != COMPLETE && message->receive_time + DATA_MESSAGE_TIMEOUT // TODO
	}
}
