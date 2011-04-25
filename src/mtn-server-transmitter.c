#include "mtn-server-transmitter.h"

#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utlist.h"
#include "uthash.h"

#include "mtn-common.h"
#include "mtn-server.h"
#include "mtn-server-data-message.h"

/*static message_list to_send_queue = NULL;
static message_list sent_map = NULL;

static pthread_mutex_t to_send_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t to_send_queue_cond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t sent_map_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sent_map_cond = PTHREAD_COND_INITIALIZER;*/

static sem_t max_sendings_semaphore;

static pthread_mutex_t sending_done_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sending_done_cond = PTHREAD_COND_INITIALIZER;
static sem_t sending_done_semaphore;

static
void
sending_done()
{
    sem_wait(&sending_done_semaphore);
    pthread_mutex_lock(&sending_done_mutex);
    if(get_sem_value(&sending_done_semaphore) == 0)
    {
        pthread_cond_signal(&sending_done_cond);
    }
    pthread_mutex_unlock(&sending_done_mutex);
}

void
transmission_control()
{
    if(sem_init(&max_sendings_semaphore, 0, MAX_ACTIVE_SENDINGS) == -1)
    {
        fatal_error_errno(1, "cannot init semaphore");
    }

    pthread_t trans_thread;
    pthread_create(&trans_thread, NULL, transmission_thread, NULL);
}

void*
transmission_thread(void* arg)
{
    pthread_t send_thread;
    while(true)
    {
        pthread_mutex_lock(to_send_queue_mutex());

        while(*to_send_queue() == NULL)
        {
            debug(5, "wait for data message to send");
            pthread_cond_wait(to_send_queue_cond(), to_send_queue_mutex());
        }

        struct client_map_node* node;
        struct client_map_node* tmp;

        pthread_mutex_lock(server_client_map_mutex());

        int clients_count = HASH_COUNT(*server_client_map());
        sem_init(&sending_done_semaphore, 0, clients_count);

        // send data message to every client
        HASH_ITER(hh, *server_client_map(), node, tmp)
        {
            if(node->data->last_update_time + SUBSCR_EXPIRE_TIME < current_time())
            {
                // remove client if subscription expired
                HASH_DEL(*server_client_map(), node);
                free(node->data);
                free(node);

                sending_done();
            }
            else
            {
                // otherwise send data message
                sem_wait(&max_sendings_semaphore);

                pthread_create(&send_thread, NULL, send_data_message_to_client, node->data);
            }
        }

        pthread_mutex_unlock(server_client_map_mutex());

        // wait until data message was send to every client
        pthread_mutex_lock(&sending_done_mutex);
        while(get_sem_value(&sending_done_semaphore) != 0)
        {
            pthread_cond_wait(&sending_done_cond, &sending_done_mutex);
        }
        pthread_mutex_unlock(&sending_done_mutex);

        sem_destroy(&sending_done_semaphore);

        // move data message to list of sent messages
        struct message_queue_node* queue_node = *to_send_queue();
        struct data_message* message = queue_node->data;

        DL_DELETE(*to_send_queue(), queue_node);
        free(node);
        pthread_mutex_unlock(to_send_queue_mutex());

        ALLOCATE(struct message_map_node, map_node);

        /*struct message_map_node* map_node =
            (struct message_map_node*) malloc(sizeof(struct message_map_node));

        if(map_node == NULL)
        {
            fatal_error(1, "cannot allocate memory");
        }

        memset(map_node, 0, sizeof(struct message_map_node));*/

        // move new message file to sent messages directory
        char src_file[4096];
        char dest_file[4096];

        strcpy(src_file, USER_NEW_MESSAGE_DIR);
        strcat(src_file, message->file_name);

        strcpy(dest_file, USER_SENT_MESSAGE_DIR);
        strcat(dest_file, message->file_name);

        char tmp_file[4096];
        int index = 0;
        strcpy(tmp_file, dest_file);
        while(file_exists(tmp_file))
        {
            ++index;
            snprintf(tmp_file, 4096, "%s-%d", dest_file, index);
        debug(5, "%s", tmp_file);

        }
        
        if(index)
        {
            strcpy(dest_file, tmp_file);
        }

        int file_fd = load_file(dest_file, O_WRONLY, src_file);
        if(file_fd != -1)
        {
            close(file_fd);

            info("moving data message file %s to sent messages directory as %s file",
                    src_file, dest_file);
        }
        else
        {
            warning("cannot move data message file %s to sent messages directory");
        }

        // remove new message source file
        if(! rm_file(src_file))
        {
            warning("some data message files are left in the new message directory ... should be removed before next new message");
        }


        map_node->data = message;
        map_node->key.time = message->release_time;
        map_node->key.id = message->id;

        pthread_mutex_lock(sent_map_mutex());
        HASH_ADD(hh, *sent_map(), key, sizeof(struct message_map_key), map_node);
        pthread_mutex_unlock(sent_map_mutex());
    }

    pthread_exit(NULL);
}

void*
send_data_message_to_client(void* arg)
{
    struct client_data* client = (struct client_data*) arg;

    struct data_message* message = (*to_send_queue())->data;

    char ip_address_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client->address.sin_addr, ip_address_str, INET_ADDRSTRLEN);

    int port = ntohs(client->address.sin_port);

    char release_time_str[10];
    strftime(release_time_str, 10, "%s", localtime(&message->release_time));

    char data_message_head[MTN_MSG_LEN + 1];
    int length = snprintf(data_message_head, MTN_MSG_LEN + 1, "%s %s %.3d %.3d",
            MTN_DM_HEAD_TXT, release_time_str, message->id, message->blocks_count);

    if(sendto(server_socket_fd(), data_message_head, length, 0,
                (struct sockaddr*) &client->address, sizeof(client->address)) == -1)
    {
        error_errno("cannot send data message to %s:%d", ip_address_str, port);
    }

    int block_number = 1;
    struct data_message_block* block;
    DL_FOREACH(message->blocks, block)
    {
        char data_message_block[MTN_MSG_LEN + 1];
        length = snprintf(data_message_block, MTN_MSG_LEN + 1, "%s %s %.3d %.3d\n%s",
                MTN_DM_BLOCK_TXT, release_time_str, message->id,
                block_number, block->data);

        if(sendto(server_socket_fd(), data_message_block, length, 0,
                    (struct sockaddr*) &client->address, sizeof(client->address)) == -1)
        {
            error_errno("cannot send data message to %s:%d", ip_address_str, port);
        }
    }

    sem_post(&max_sendings_semaphore);

    sending_done();

    pthread_exit(NULL);
}
