#ifndef MTN_COMMON_H
#define MTN_COMMON_H

#include "mtn-protocol.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define LINE_MAX_SIZE 4096
#define TOKEN_MAX_SIZE 4096

#define USER_CFG_PATH "~/.config/mtn/server/"

#define USER_SERVER_CFG_FILE "server.conf"

#define USER_CLIENT_CFG_FILE "client.conf"
#define USER_CLIENT_SERVER_LIST_FILE "client/server_list"

#define EXAMPLE_SERVER_CFG_FILE "server.conf.example"
#define EXAMPLE_CLIENT_CFG_FILE "client.conf.example"

#define SERVER_DEFAULT_PORT 48888
#define SERVER_DEFAULT_SET 60

#define SERVER_RESPONSE_TIMEOUT 30

struct msg_id
{
    uint32_t message;
    uint32_t block;
};

struct mtn_request
{
    mtn_request_type type;
    union {
        struct msg_id msg;
    } info;
};

int strcopy(char* dest, size_t* dest_position, const char* src, int copy_len);

/**
 * Replaces first occurance of substring in a string
 * with another string
 *
 * All input string have to end with '\0'
 *
 * @param[in] what what to replace
 * @param[in] with replace with what
 * @param[in] in source string
 * @param[out] where where the result come
 */
void str_replace(const char* what, const char* with, const char* in, char* where);

void init_dir(char* path);
void init_file(char* file_path, const char* example_file);

int get_file_line(char* line, int conf_file_fd, bool nonempty);
int get_token(char* token, const char* str, const char* delim, bool from_beginning);

#endif
