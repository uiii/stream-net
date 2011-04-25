#include "mtn-protocol.h"

#include <string.h>

#include "mtn-common.h"

mtn_message_type
get_message_type(const char* message)
{
    mtn_message_type type = MTN_UNKNOWN;

    if(strneq(message, MTN_REQUEST_TXT, MTN_MSG_TYPE_LEN))
    {
        type = MTN_REQUEST;
    }
    else if(strneq(message, MTN_RESPONSE_TXT, MTN_MSG_TYPE_LEN))
    {
        type = MTN_RESPONSE;
    }
    else if(strneq(message, MTN_DM_HEAD_TXT, MTN_MSG_TYPE_LEN))
    {
        type = MTN_DM_HEAD;
    }
    else if(strneq(message, MTN_DM_BLOCK_TXT, MTN_MSG_TYPE_LEN))
    {
        type = MTN_DM_BLOCK;
    }

    return type;
}

mtn_request_type
get_request_type(const char* message)
{
    const char* message_ptr = message + MTN_MSG_TYPE_LEN + 1;

    mtn_request_type type = MTN_REQ_UNKNOWN;

    if(strneq(message_ptr, MTN_REQ_SUBSCR_TXT, MTN_REQ_TYPE_LEN))
    {
        type = MTN_REQ_SUBSCR;
    }
    else if(strneq(message_ptr, MTN_REQ_RESEND_TXT, MTN_REQ_TYPE_LEN))
    {
        type = MTN_REQ_RESEND;
    }

    return type;
}

mtn_response_type
get_response_type(const char* message)
{
    const char* message_ptr = message + MTN_MSG_TYPE_LEN + 1;

    mtn_response_type type = MTN_RES_UNKNOWN;

    if(strneq(message_ptr, MTN_RES_ACCEPT_TXT, MTN_RES_TYPE_LEN))
    {
        type = MTN_RES_ACCEPT;
    }
    else if(strneq(message_ptr, MTN_RES_REJECT_TXT, MTN_RES_TYPE_LEN))
    {
        type = MTN_RES_REJECT;
    }

    return type;
}

void
get_message_type_str(char* type_str, mtn_message_type type)
{
    switch(type)
    {
        case MTN_REQUEST:
            strcpy(type_str, MTN_REQUEST_TXT);
            break;
        case MTN_RESPONSE:
            strcpy(type_str, MTN_RESPONSE_TXT);
            break;
        case MTN_DM_HEAD:
            strcpy(type_str, MTN_DM_HEAD_TXT);
            break;
        case MTN_DM_BLOCK:
            strcpy(type_str, MTN_DM_BLOCK_TXT);
            break;
        default:
            strcpy(type_str, "UNKNOWN");
            break;
    }
}
void
get_request_type_str(char* type_str, mtn_request_type type)
{
    switch(type)
    {
        case MTN_REQ_SUBSCR:
            strcpy(type_str, MTN_REQ_SUBSCR_TXT);
            break;
        case MTN_REQ_RESEND:
            strcpy(type_str, MTN_REQ_RESEND_TXT);
            break;
        default:
            strcpy(type_str, "UNKNOWN");
            break;
    }
}

void
get_response_type_str(char* type_str, mtn_response_type type)
{
    switch(type)
    {
        case MTN_RES_ACCEPT:
            strcpy(type_str, MTN_RES_ACCEPT_TXT);
            break;
        case MTN_RES_REJECT:
            strcpy(type_str, MTN_RES_REJECT_TXT);
            break;
        default:
            strcpy(type_str, "UNKNOWN");
            break;
    }
}
