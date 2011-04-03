#include "common.h"

mtn_request_type
get_request_type(const char* req_buf, size_t req_len)
{
    mtn_request_type type = MTN_REQ_UNKNOWN;

    if(MATCH_REQ(req_buf, req_len, MTN_REQ_SUBSCR_TXT))
    {
        type = MTN_REQ_SUBSCR;
    }
    else if(MATCH_REQ(req_buf, req_len, MTN_REQ_UPDATE_TXT))
    {
        type = MTN_REQ_UPDATE;
    }
    else if(MATCH_REQ(req_buf, req_len, MTN_REQ_RESEND_TXT))
    {
        type = MTN_REQ_RESEND;
    }

    return type;
}

int
strcopy(char* dest, size_t* dest_position, const char* src, int copy_len)
{
    int position = *dest_position;
    int i = 0;
    for(i = 0; src[i] != '\0'; ++i)
    {
        dest[position + i] = src[i];

        if(copy_len > -1 && (i == copy_len))
        {
            break;
        }
    }

    *dest_position = position + i;
    return i;
}

void str_replace(const char* what, const char* with, const char* in, char* where)
{
    char* substr = strstr(in, what);

    if((substr != NULL) && (what != NULL))
    {
        int pos = 0;
        int prefix_len = substr - in;
        strcopy(where, &pos, in, prefix_len);

        if(with != NULL)
        {
            strcopy(where, &pos, with, -1);
        }

        char* sufix = substr + strlen(what);
        strcopy(where, &pos, sufix, -1);
    }
    else
    {
        strcpy(where, in);
    }
}
