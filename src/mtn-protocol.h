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

#define MTN_REQ_MSG_LEN 32 // length of client request message
#define MTN_RES_MSG_LEN 128 // length of server response to request message
#define MTN_MSG_BLOCK_LEN 1024 // length of message block

#define SRV_NAME_LEN 128 // max length of server name
#define SRV_SET_LEN 8 // max length of subscription expiration time
#define SRV_DESCR_LEN 256 // max length of server description

/* protocol commands client -> server */
#define MTN_REQ_SUBSCR_TXT "SUBSCR" // request for subscription
#define MTN_REQ_UPDATE_TXT "UPDATE" // request for subscription
#define MTN_REQ_RESEND_TXT "RESEND" // request for message block resending

/* protocol commands server -> client */
#define MTN_RES_TXT "" // TODO

#define MATCH_REQ(req_buf, req_len, req_type_txt) \
    ((req_len) >= strlen(req_type_txt) \
        && strncmp(req_buf, req_type_txt, strlen(req_type_txt)) == 0)

typedef enum
{
    MTN_REQ_SUBSCR,
    MTN_REQ_UPDATE,
    MTN_REQ_RESEND,
    MTN_REQ_UNKNOWN
} mtn_request_type;

#endif
