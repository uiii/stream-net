/*
 * This file is part of the Message Transmission Net project.
 *
 * Copyright (c) 2011 Richard Jedlička <uiii.dev@gmail.com>. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL RICHARD JEDLIČKA
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Richard Jedlička.
 */

#ifndef MTN_PROTOCOL_H
#define MTN_PROTOCOL_H

/*
 * MTNP protocol
 *
 * (Message transmission net)
 */

#define MTN_MSG_LEN 1024

#define MTN_MSG_TYPE_LEN 4
#define MTN_REQ_TYPE_LEN 6 // length of client request type
#define MTN_RES_TYPE_LEN 6 // length of server response type

//#define MTN_REQ_DATA_LEN 32 // length of client request data
//#define MTN_RES_DATA_LEN 128 // length of server response data


//#define MTN_REQ_MSG_LEN (MTN_MSG_TYPE_LEN + MTN_REQ_TYPE_LEN + MTN_REQ_DATA_LEN) // length of client request message
//#define MTN_RES_MSG_LEN (MTN_MSG_TYPE_LEN + MTN_RES_TYPE_LEN + MTN_RES_DATA_LEN) // length of server response to request message

#define MTN_DM_HEAD_DATA_LEN (10 + 3 + 3 + 2) // date in seconds + id + blocks count + spaces

#define MTN_DM_BLOCK_HEAD_LEN (10 + 3 + 3 + 2) // length of data message block's head (date in seconds + id + number of block + spaces)
#define MTN_DM_BLOCK_DATA_LEN (MTN_MSG_LEN - MTN_MSG_TYPE_LEN - MTN_DM_BLOCK_HEAD_LEN) // length of data message block

//#define MTN_REQUEST_LEN (MTN_MSG_TYPE_LEN + MTN_REQUEST_TYPE_LEN + MTN_

// length of data message's head
//#define MTN_DM_HEAD_LEN (MTN_MSG_TYPE_LEN + 10 + 3 + 3 + 3) 

//#define MTN_DM_BLOCK_LEN (MTN_MSG_TYPE_LEN + 10 + 3 + 3 + 3) 

#define SRV_NAME_LEN 128 // max length of server name
#define SRV_DESCR_LEN 256 // max length of server description

#define MTN_REQUEST_TXT "RQST"
#define MTN_RESPONSE_TXT "RSPS"
#define MTN_DM_HEAD_TXT "DATA"
#define MTN_DM_BLOCK_TXT "BLCK"

/* protocol commands client -> server */
#define MTN_REQ_SUBSCR_TXT "SUBSCR" // request for subscription
#define MTN_REQ_RESEND_TXT "RESEND" // request for message block resending

/* protocol commands server -> client */
#define MTN_RES_ACCEPT_TXT "ACCEPT" // request for message block resending
#define MTN_RES_REJECT_TXT "REJECT" // request for message block resending

#define MATCH_STR(first_buf, req_len, req_type_txt) \
    ((req_len) >= strlen(req_type_txt) \
        && strncmp(req_buf, req_type_txt, strlen(req_type_txt)) == 0)

typedef enum
{
    MTN_REQ_SUBSCR,
    MTN_REQ_RESEND,
    MTN_REQ_UNKNOWN
} mtn_request_type;

typedef enum
{
    MTN_RES_ACCEPT,
    MTN_RES_REJECT,
    MTN_RES_UNKNOWN
} mtn_response_type;

typedef enum
{
    MTN_REQUEST,
    MTN_RESPONSE,
    MTN_DM_HEAD,
    MTN_DM_BLOCK,
    MTN_UNKNOWN
} mtn_message_type;

mtn_message_type get_message_type(const char* message);
mtn_request_type get_request_type(const char* message);
mtn_response_type get_response_type(const char* message);

void get_message_type_str(char* message_type_str, mtn_message_type);
void get_request_type_str(char* request_type_str, mtn_request_type);
void get_response_type_str(char* response_type_str, mtn_response_type);

#endif
