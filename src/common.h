#ifndef MTN_COMMON_H
#define MTN_COMMON_H

#include "mtn-protocol.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

mtn_request_type get_request_type(const char* req_buf, size_t req_len);

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

#endif
