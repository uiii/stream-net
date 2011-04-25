#include "mtn-server-data-message.h"

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utlist.h"
#include "uthash.h"

#include "mtn-common.h"

message_queue*
to_send_queue()
{
    static message_queue queue = NULL;
    return &queue;
}

pthread_mutex_t*
to_send_queue_mutex()
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    return &mutex;
}

pthread_cond_t*
to_send_queue_cond()
{
    static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    return &cond;
}

message_map*
sent_map()
{
    static message_map map = NULL;
    return &map;
}

pthread_mutex_t*
sent_map_mutex()
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    return &mutex;
}

pthread_cond_t*
sent_map_cond()
{
    static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    return &cond;
}

static bool new_message_available = false;
static pthread_mutex_t new_message_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t new_message_cond = PTHREAD_COND_INITIALIZER;

void
new_message_signal(int arg)
{
    pthread_mutex_lock(&new_message_mutex);
    new_message_available = true;
    pthread_cond_signal(&new_message_cond);
    pthread_mutex_unlock(&new_message_mutex);

    info("new message available");
}

void
data_message_control()
{
    pthread_t thread;
    pthread_create(&thread, NULL, load_data_message_thread, NULL);
}

void*
load_data_message_thread(void* arg)
{
    while(true)
    {
        pthread_mutex_lock(&new_message_mutex);
        while(! new_message_available)
        {
            pthread_cond_wait(&new_message_cond, &new_message_mutex);
        }
        new_message_available = false;
        pthread_mutex_unlock(&new_message_mutex);

        DIR* dir = load_dir(USER_NEW_MESSAGE_DIR);

        if(dir == NULL)
        {
            warning("no new messages will be loaded");
        }
        else
        {
            struct dirent* dir_file;

            int message_count = 0;

            errno = 0;
            while((dir_file = readdir(dir)) != NULL)
            {
                if(streq(dir_file->d_name, ".") || streq(dir_file->d_name, ".."))
                {
                    continue;
                }

                char path[4096];
                strcpy(path, USER_NEW_MESSAGE_DIR);
                strcat(path, dir_file->d_name);

                // load message from file
                int message_file_fd = load_file(path, O_RDONLY, NULL);
                if(message_file_fd != -1)
                {
                    ALLOCATE(struct data_message, message);

                    /*struct data_message* message =
                        (struct data_message*) malloc(sizeof(struct data_message));

                    if(message == NULL)
                    {
                        fatal_error(1, "cannot allocate memory");
                    }

                    memset(message, 0, sizeof(message));*/

                    int read_len;
                    char buffer[MTN_DM_BLOCK_DATA_LEN + 1];
                    while((read_len = read(message_file_fd, buffer, MTN_DM_BLOCK_DATA_LEN)) > 0)
                    {
                        buffer[read_len] = '\0';
                        
                        ALLOCATE(struct data_message_block, block);

                        /*struct data_message_block* block =
                            (struct data_message_block*) malloc(sizeof(struct data_message_block));

                        if(block == NULL)
                        {
                            fatal_error(1, "cannot allocate memory");
                        }
                         
                        memset(block, 0, sizeof(block));*/

                        strcpy(block->data, buffer);

                        DL_APPEND(message->blocks, block);
                        message->blocks_count++;
                    }
                    
                    if(read_len == -1)
                    {
                        error_errno("cannot read data message file %s", path);
                    }

                    if(message->blocks_count == 0)
                    {
                        warning("data message from file %s is empty ... IGNORING", path);
                        free(message);
                    }
                    else
                    {
                        static int last_id = 0;
                        static time_t last_release_time = 0;

                        message->release_time = current_time();
                        if(message->release_time == last_release_time)
                        {
                            message->id = last_id + 1;
                        }
                        else
                        {
                            message->id = 0;
                        }

                        last_id = message->id;
                        last_release_time = message->release_time;

                        strcpy(message->file_name, dir_file->d_name);

                        ALLOCATE(struct message_queue_node, node);
                        node->data = message;

                        pthread_mutex_lock(to_send_queue_mutex());

                        DL_APPEND(*to_send_queue(), node);
                        pthread_cond_broadcast(to_send_queue_cond());

                        pthread_mutex_unlock(to_send_queue_mutex());
                        
                        info("loading data message from file %s", path);

                        message_count++;
                    }

                    close(message_file_fd);
                }

                errno = 0;
            }

            if(errno != 0)
            {
                error_errno("cannot read whole %s directory", USER_NEW_MESSAGE_DIR);
                warning("some new messages might not be loaded");
            }
            
            if(message_count)
            {
                info("%d new data message%s loaded", message_count, (message_count > 1) ? "s" : "");
            }
            else
            {
                warning("no new data messages loaded");
            }

            closedir(dir);
        }
    }

    pthread_exit(NULL);
}
