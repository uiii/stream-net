#include "mtn-client-updater.h"

#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "array_heap.h"

#include "mtn-client.h"
#include "mtn-client-request.h"

//static client_server_list subscribed_list = NULL;
//static client_server_list not_subscribed_list = NULL;

static
bool
connect_sooner(void* a, void* b)
{
    struct server_data* first = (struct server_data*) a;
    struct server_data* second = (struct server_data*) b;

    if(first->not_connect_count < second->not_connect_count)
    {
        return true;
    }
    else if(first->not_connect_count == second->not_connect_count)
    {
        if(first->last_connect_attempt_time <= second->last_connect_attempt_time)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

static UT_icd icd = { sizeof(struct server_data*), NULL, NULL, NULL };

static array_heap subscribed_queue = { NULL, connect_sooner };
static array_heap not_subscribed_queue = { NULL, connect_sooner };

static pthread_mutex_t subscribed_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t not_subscribed_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t subscribed_queue_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_subscribed_queue_cond = PTHREAD_COND_INITIALIZER;

static sem_t semaphore;

void
update_control()
{
    utarray_new(subscribed_queue.array, &icd);
    utarray_new(not_subscribed_queue.array, &icd);

    struct server_map_node* map_node;
    struct server_map_node* tmp_map_node;
    HASH_ITER(hh, *client_server_map(), map_node, tmp_map_node)
    {
        utarray_push_back(not_subscribed_queue.array, &map_node->data);
    }

    heap_build(not_subscribed_queue);

    if(sem_init(&semaphore, 0, MAX_ACTIVE_REQUESTS) == -1)
    {
        fatal_error_errno(1, "cannot init semaphore");
    }

    pthread_t subscr_t;
    pthread_t update_t;

    pthread_create(&subscr_t, NULL, subscription_thread, NULL);
    pthread_create(&update_t, NULL, update_thread, NULL);
}

void*
send_subscription_request_thread(void* server_data)
{
    struct server_data* server = (struct server_data*) server_data;

    char request[MTN_MSG_LEN + 1];
    int request_length = snprintf(request, MTN_MSG_LEN + 1, "%s %s",
            MTN_REQUEST_TXT, MTN_REQ_SUBSCR_TXT);
    if(send_request(server, request, request_length))
    {
        pthread_mutex_lock(&server->mutex);

        if(get_response_type(server->response) == MTN_RES_ACCEPT)
        {
            // server accepts client request
            // client is now subscribed
            server->state = SUBSCR;

            char* response_ptr = server->response;

            char line[4096];

            // skip first line
            get_token(line, &response_ptr, "\n");

            while(get_token(line, &response_ptr, "\n"))
            {
                char* line_ptr = line;

                char token[4096];
                get_token(token, &line_ptr, " ");

                if(streq(token, "name"))
                {
                    strcpy(server->name, "");
                    strcat(server->name, line_ptr + 1);
                }
                else if(streq(token, "descr"))
                {
                    strcpy(server->descr, "");
                    strcat(server->descr, line_ptr + 1);
                }
            }

            info("server->name '%s'", server->name);
            info("server->descr '%s'", server->descr);
        }
        else
        {
            // server rejects client request
            // client waits for next attempt
            server->state = WAIT;
        }

        strcpy(server->response, "");

        pthread_mutex_unlock(&server->mutex);
    }

    if(server->state == SUBSCR)
    {
        pthread_mutex_lock(&subscribed_queue_mutex);

        heap_push(subscribed_queue, &server);
        pthread_cond_broadcast(&subscribed_queue_cond);

        pthread_mutex_unlock(&server->mutex);

        pthread_mutex_unlock(&subscribed_queue_mutex);
    }
    else
    {
        pthread_mutex_lock(&not_subscribed_queue_mutex);

        heap_push(not_subscribed_queue, &server);
        pthread_cond_broadcast(&not_subscribed_queue_cond);
        
        pthread_mutex_unlock(&not_subscribed_queue_mutex);
    }

    // "unlock" semaphore
    sem_post(&semaphore);

    pthread_exit(NULL);
}

void*
subscription_thread(void* arg)
{
    pthread_t sender_thread;
    while(true)
    {
        pthread_mutex_lock(&not_subscribed_queue_mutex);

        while(heap_is_empty(not_subscribed_queue))
        {
            debug(5, "wait for not subscribed server");
            pthread_cond_wait(&not_subscribed_queue_cond, &not_subscribed_queue_mutex);
        }

        bool wait = true;
        struct timespec timeout;
        struct server_data* server = *(struct server_data**) heap_first(not_subscribed_queue);
       
        while(wait && server->state != SEND_REQ
                && (server->state != WAIT || server->last_connect_attempt_time + NEXT_CONNECT_ATTEMPT_TIME > current_time()))
        {
            timeout.tv_sec = server->last_connect_attempt_time + NEXT_CONNECT_ATTEMPT_TIME;
            timeout.tv_nsec = 0;

            if(pthread_cond_timedwait(&not_subscribed_queue_cond, &not_subscribed_queue_mutex, &timeout) == ETIMEDOUT)
            {
                wait = false;
            }

            server = *(struct server_data**) heap_first(not_subscribed_queue);
        }

        heap_pop(not_subscribed_queue);

        // client has an attempt to send
        // subscription request to the server
        server->state = SEND_REQ;

        pthread_mutex_unlock(&not_subscribed_queue_mutex);

        // TODO
        // "lock" semaphore
        sem_wait(&semaphore);

        pthread_create(&sender_thread, NULL, send_subscription_request_thread, server);
    }

    // TODO
    pthread_exit(NULL);
}

void*
update_thread(void* arg)
{
    pthread_t sender_thread;
    while(true)
    {
        pthread_mutex_lock(&subscribed_queue_mutex);

        while(heap_is_empty(subscribed_queue))
        {
            debug(5, "wait for subscribed server");
            pthread_cond_wait(&subscribed_queue_cond, &subscribed_queue_mutex);
        }

        bool wait = true;
        struct timespec timeout;
        struct server_data* server = *(struct server_data**) heap_first(subscribed_queue);
        while(wait && (server->last_connect_attempt_time + SUBSCR_EXPIRE_TIME) > current_time())
        {
            timeout.tv_sec = server->last_connect_attempt_time + SUBSCR_EXPIRE_TIME;
            timeout.tv_nsec = 0;

            if(pthread_cond_timedwait(&subscribed_queue_cond, &subscribed_queue_mutex, &timeout) == ETIMEDOUT)
            {
                wait = false;
            }

            server = *(struct server_data**) heap_first(subscribed_queue);
        }

        heap_pop(subscribed_queue);

        pthread_mutex_unlock(&subscribed_queue_mutex);

        // TODO
        // "lock" semaphore
        sem_wait(&semaphore);

        info("update");
        pthread_create(&sender_thread, NULL, send_subscription_request_thread, server);
    }

    // TODO
    pthread_exit(NULL);
}
