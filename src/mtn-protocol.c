#include "mtn-protocol.h"

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

void
get_request_type_str(char* req_type_str, const char* req_buf)
{
    strncpy(req_type_str, req_buf, MTN_REQ_TYPE_LEN);
}

mtn_response_type
get_response_type(const char* res_buf, size_t res_len)
{
    mtn_response_type type = MTN_REQ_UNKNOWN;

    if(MATCH_REQ(res_buf, res_len, MTN_REQ_SUBSCR_TXT))
    {
        type = MTN_REQ_SUBSCR;
    }
    else if(MATCH_REQ(res_buf, res_len, MTN_REQ_UPDATE_TXT))
    {
        type = MTN_REQ_UPDATE;
    }
    else if(MATCH_REQ(res_buf, res_len, MTN_REQ_RESEND_TXT))
    {
        type = MTN_REQ_RESEND;
    }

    return type;
}

void
get_response_type_str(char* res_type_str, const char* res_buf)
{
    strncpy(res_type_str, res_buf, MTN_REQ_TYPE_LEN);
}
