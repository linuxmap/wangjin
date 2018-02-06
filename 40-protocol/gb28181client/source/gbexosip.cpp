#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#include <time.h>
#else
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#endif




// prj include
#include "oscbb.h"

#include "common.h"
#include "gb.h"
#include "gbexosip.h"

using namespace GB;




const char *CGbExosip::AUTH_TYPE                    = "Digest";
const char *CGbExosip::AUTH_ALGORITHM               = "MD5";

const char *CGbExosip::CONTENT_TYPE_APPLICATION     = "application";
const char *CGbExosip::CONTENT_SUBTYPE_MANSCDP_XML  = "MANSCDP+xml";
const char *CGbExosip::CONTENT_SUBTYPE_SDP          = "sdp";

// deal message
void CGbExosip::deal_response()
{
    osip_message_t *msg = recv_msg;
    if (!strncasecmp(msg->cseq->method, GBSIP_METHOD_REGISTER, strlen(GBSIP_METHOD_REGISTER)))
    {
        /// 需要权???
        if (SIP_RESPONSE_401_UNAUTHORIZED == msg->status_code)
        {
            call_back(EV_TYPE_RES_UNAUTH_REG, msg, sizeof(msg), rmt_ip, rmt_port, cur_time);
        }
        /// 成功认证
        else if (SIP_RESPONSE_200_OK == msg->status_code)
        {
            call_back(EV_TYPE_RES_AUTH_REG, msg, sizeof(msg), rmt_ip, rmt_port, cur_time);
        }
        else
        {
            CLog::log(CLog::CLOG_LEVEL_GB_TASK, "sip response REGISTER status code:%u\n", msg->status_code);
        }
    }
    else if (!strncasecmp(msg->cseq->method, GBSIP_METHOD_INVITE, strlen(GBSIP_METHOD_INVITE)))
    {
        call_back(EV_TYPE_RES_INVITE, msg, sizeof(msg), rmt_ip, rmt_port, cur_time);
	}
}


// deal message
void CGbExosip::deal_request()
{
    osip_message_t *msg = recv_msg;

    if (!strncasecmp(msg->sip_method, GBSIP_METHOD_REGISTER, strlen(GBSIP_METHOD_REGISTER)))
    {
        // deal_register();
    }
    else if (!strncasecmp(msg->sip_method, GBSIP_METHOD_MESSAGE, strlen(GBSIP_METHOD_MESSAGE)))
    {
        deal_message();
    }
    else if (!strncasecmp(msg->sip_method, GBSIP_METHOD_REGISTER, strlen(GBSIP_METHOD_REGISTER)))
    {
        // deal_invite();
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "invalid cmd:%s\n", msg->sip_method);
        // gbsip_reply_error(c, SIP_RESPONSE_400_BAD_REQ);
    }
}


/// 应答 MESSAGE 消息
int CGbExosip::res_to_message(osip_message_t *req, ESipResCode code, char *tag, uint32_t tag_len)
{
    osip_message_t *answer  = NULL;

    /// response
    int ret = build_response(&answer, code, req);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "message answer build failed:%d\n", ret);
        return -1;
    }

    if (tag)
    {
        osip_generic_param_t *t;
        osip_from_get_tag (req->from, &t);
        snprintf(tag, tag_len, "%s", t->gvalue);
    }

    /// send response to peer
    ret = send_data(answer);
    osip_free(answer);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send message answer failed:%d\n", ret);
        return ret;
    }

    return 0;
}

int CGbExosip::res_to_message(osip_message_t *req, ESipResCode code, uint32_t ip, uint16_t port, char *tag, uint32_t tag_len)
{
    osip_message_t *answer = NULL;

    /// response
    int ret = build_response(&answer, code, req);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "message answer build failed:%d\n", ret);
        return -1;
    }

    if (tag)
    {
        osip_generic_param_t *t;
        osip_from_get_tag(req->from, &t);
        snprintf(tag, tag_len, "%s", t->gvalue);
    }

    /// send response to peer
    ret = send_data(answer, ip, port );
    osip_free(answer);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send message answer failed:%d\n", ret);
        return ret;
    }

    return 0;
}

#define SIP_ADD(fmt, ...) snprintf(send_buf + send_len, sizeof(send_buf) - send_len, fmt, ##__VA_ARGS__);   \
                            send_len = strlen(send_buf)
/// Ӧ??ý??????
int CGbExosip::ack_invite(char *media_sender_id, char *dst_domain, char *src_id, char *src_domain,
                        uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                        char *via, char *call_id, char *from, char *to, char *cseq)
{
    int ret;
    send_len    = 0;
    SIP_ADD("ACK sip:%s@%s SIP/2.0\r\n", media_sender_id, dst_domain);
    SIP_ADD("Via: %s\r\n", via);
    SIP_ADD("Call-ID: %s\r\n", call_id);
    SIP_ADD("From: %s\r\n", from);
    SIP_ADD("To: %s\r\n", to);
    SIP_ADD("CSeq: %s\r\n", cseq);

    unsigned char *bytes = (unsigned char *)&src_ip;

    SIP_ADD("Contact: <sip:%s@%u.%u.%u.%u:%u>\r\n", src_id, bytes[0], bytes[1], bytes[2], bytes[3], src_port);
    SIP_ADD("Max-Forwards: 70\r\nContent-Length: 0\r\n\r\n");

    // ack media invite
    ret = send_data(send_buf, send_len, dst_ip, dst_port);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send ack request failed,dev:%s, ip:%x,port:%u\n",
                    media_sender_id, dst_ip, dst_port);
    }

    return ret;
}

/// INVITE method
int CGbExosip::invite_media(char *media_sender_id, char *dst_domain, char *src_id, char *src_domain,
                        uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                        uint32_t seq, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype,
                        char **local_tag, char **call_id)
{
    int ret;
    char *tag;
    send_len    = 0;
    SIP_ADD("INVITE sip:%s@%s SIP/2.0\r\n", media_sender_id, dst_domain);
    SIP_ADD("Via: SIP/2.0/UDP %s;branch=z9hG4bK%u\r\n", src_id, osip_build_random_number());

    /// set call-id
    tag = malloc_new_random();
    if (tag)
    {
        SIP_ADD("Call-ID: %s\r\n", tag);
        if (call_id)
        {
            *call_id    = tag;
        }
        else
        {
            osip_free(tag);
        }
    }
    else
    {
        return GB_ERR_NOMEM;
    }

    /// set local-tag
    tag   = malloc_new_random();
    if (tag)
    {
        SIP_ADD("From: <sip:%s@%s>;tag=%s\r\n", src_id, src_domain, tag);
        if (local_tag)
        {
            *local_tag  = tag;
        }
        else
        {
            osip_free(tag);
        }
    }
    else
    {
        return GB_ERR_NOMEM;
    }

    SIP_ADD("To: <sip:%s@%s>\r\n", media_sender_id, dst_domain);
    SIP_ADD("CSeq: %u INVITE\r\n", seq);


    unsigned char *bytes = (unsigned char *)&src_ip;
    SIP_ADD("Contact: <sip:%s@%u.%u.%u.%u:%u>\r\n", src_id, bytes[0], bytes[1], bytes[2], bytes[3], src_port);

    char *psz_sn   = malloc_new_random();
    char *psz_sn1  = malloc_new_random();
    SIP_ADD("Subject: %s:0%s,%s:0%s\r\n", media_sender_id, psz_sn, src_id, psz_sn1);
    osip_free(psz_sn);
    osip_free(psz_sn1);

    SIP_ADD("Max-Forwards: 70\r\n");
    SIP_ADD("Content-Type: application/sdp\r\n");

    char sdp_buf[256];
    int sdp_len = 0;
    snprintf(sdp_buf, sizeof(sdp_buf), "v=0\r\no=%s 0 0 IN IP4 %u.%u.%u.%u\r\n",
                src_id, bytes[0], bytes[1], bytes[2], bytes[3]);

    sdp_len = strlen(sdp_buf);
    snprintf(sdp_buf + sdp_len, sizeof(sdp_buf) - sdp_len, "s=Play\r\nc=IN IP4 %u.%u.%u.%u\r\nt=0 0\r\n",
                bytes[0], bytes[1], bytes[2], bytes[3]);

    sdp_len = strlen(sdp_buf);
    snprintf(sdp_buf + sdp_len, sizeof(sdp_buf) - sdp_len, "m=video %u RTP/AVP %u\r\na=recvonly\r\n",
                media_recv_port, ptype);

    sdp_len = strlen(sdp_buf);
    snprintf(sdp_buf + sdp_len, sizeof(sdp_buf) - sdp_len, "a=rtpmap:%u PS/90000\r\ny=%u\r\n", ptype, ssrc);
    sdp_len = strlen(sdp_buf);

    SIP_ADD("Content-Length: %u\r\n\r\n", sdp_len);
    SIP_ADD("%s", sdp_buf);

    /// send data by linker
    ret = send_data(send_buf, send_len, dst_ip, dst_port);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send INVITE request failed,dev:%s, ip:%x,port:%u\n",
                    media_sender_id, dst_ip, dst_port);
    }

    return ret;
}

/// BYE media op
int CGbExosip::bye_media(char *media_sender_id, char *dst_domain, char *src_id, char *src_domain,
                            uint32_t dst_ip, uint16_t dst_port, uint32_t seq,
                            char *call_id, char *from, char *to)
{
    int ret;
    send_len    = 0;
    SIP_ADD("BYE sip:%s@%s SIP/2.0\r\n", media_sender_id, dst_domain);
    SIP_ADD("Via: SIP/2.0/UDP %s;branch=z9hG4bK%u\r\n", src_id, osip_build_random_number());
        SIP_ADD("Call-ID: %s\r\n", call_id);
    SIP_ADD("From: <sip:%s@%s>;tag=%s\r\n", src_id, src_domain, from);
    SIP_ADD("To: <sip:%s@%s>;tag=%s\r\n", media_sender_id, dst_domain, to);
    SIP_ADD("CSeq: %u BYE\r\n", seq);

    /// unsigned char *bytes = (unsigned char *)&src_ip;
    /// SIP_ADD("Contact: <sip:%s@%u.%u.%u.%u:%u>\r\n", src_id, bytes[0], bytes[1], bytes[2], bytes[3], src_port);

    SIP_ADD("Max-Forwards: 70\r\nContent-Length: 0\r\n\r\n");

    // ack media invite
    ret = send_data(send_buf, send_len, dst_ip, dst_port);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send ack request failed,dev:%s, ip:%x,port:%u\n",
                    media_sender_id, dst_ip, dst_port);
    }

    return ret;
}


int CGbExosip::build_response(osip_message_t **dest, int status, osip_message_t *request, char *user_agent)
{
    osip_generic_param_t *tag;
    osip_message_t *response;
    int i;

    *dest = NULL;
    if (request == NULL)
      return OSIP_BADPARAMETER;

    i = osip_message_init (&response);
    if (i != 0)
      return i;

    response->sip_version = (char *) osip_malloc (8 * sizeof (char));
    if (response->sip_version == NULL) {
        osip_message_free (response);
        return OSIP_NOMEM;
    }
    sprintf (response->sip_version, "SIP/2.0");
    osip_message_set_status_code (response, status);

    /* handle some internal reason definitions. */
    if (MSG_IS_NOTIFY(request) && status == 481) {
        response->reason_phrase = osip_strdup("Subscription Does Not Exist");
    }
    else if (MSG_IS_SUBSCRIBE (request) && status == 202) {
        response->reason_phrase = osip_strdup("Accepted subscription");
    }
    else {
        response->reason_phrase = osip_strdup(osip_message_get_reason (status));
        if (response->reason_phrase == NULL) {
            if (response->status_code == 101)
              response->reason_phrase = osip_strdup ("Dialog Establishement");
            else
              response->reason_phrase = osip_strdup ("Unknown code");
        }
        response->req_uri = NULL;
        response->sip_method = NULL;
    }

    if (response->reason_phrase == NULL) {
        osip_message_free (response);
        return OSIP_NOMEM;
    }

    i = osip_to_clone (request->to, &(response->to));
    if (i != 0) {
        osip_message_free (response);
        return i;
    }

    i = osip_to_get_tag (response->to, &tag);
    if (i != 0) {                 /* we only add a tag if it does not already contains one! */
        if (status != 100)
          osip_to_set_tag (response->to, malloc_new_random ());
    }

    i = osip_from_clone (request->from, &(response->from));
    if (i != 0) {
        osip_message_free (response);
        return i;
    }

    {
        osip_list_iterator_t it;
        osip_via_t *via = (osip_via_t*)osip_list_get_first(&request->vias, &it);

        while (via != NULL) {
            osip_via_t *via2;

            i = osip_via_clone (via, &via2);
            if (i != 0) {
                osip_message_free (response);
                return i;
            }
            osip_list_add (&response->vias, via2, -1);
            via = (osip_via_t *)osip_list_get_next(&it);
        }
    }

    i = osip_call_id_clone (request->call_id, &(response->call_id));
    if (i != 0) {
        osip_message_free (response);
        return i;
    }
    i = osip_cseq_clone (request->cseq, &(response->cseq));
    if (i != 0) {
        osip_message_free (response);
        return i;
    }

    if (MSG_IS_SUBSCRIBE (request)) {
        osip_header_t *exp;
        osip_header_t *evt_hdr;

        osip_message_header_get_byname (request, "event", 0, &evt_hdr);
        if (evt_hdr != NULL && evt_hdr->hvalue != NULL)
          osip_message_set_header (response, "Event", evt_hdr->hvalue);
        else
          osip_message_set_header (response, "Event", "presence");
        i = osip_message_get_expires (request, 0, &exp);
        if (exp == NULL) {
            osip_header_t *cp;

            i = osip_header_clone (exp, &cp);
            if (cp != NULL)
              osip_list_add (&response->headers, cp, 0);
        }
    }

    if (user_agent)
    {
        osip_message_set_user_agent (response, user_agent);
    }
    else
    {
        osip_message_set_user_agent (response, "NETMARCH/IMOS");
    }

    *dest = response;
    return OSIP_SUCCESS;
}


/// 应答 设备信息查询 消息
int CGbExosip::res_to_dev_info(osip_message_t *req,
                                char *dst_id, char *dst_domain,
                                uint32_t dst_ip, uint16_t dst_port,
                                char *src_id, char *src_domain,
                                char *tag, char *msg_body)
{
    send_len    = 0;
    unsigned char *bytes = (unsigned char *)&dst_ip;

    SIP_ADD("MESSAGE sip:%s@%s SIP/2.0\r\n", src_id, src_domain);
    SIP_ADD("Via: SIP/2.0/UDP %u.%u.%u.%u:%u;rport;branch=z9hG4bK%u\r\n",
                bytes[0], bytes[1], bytes[2], bytes[3], dst_port, osip_build_random_number());
    SIP_ADD("From: <sip:%s@%s;tag=%s\r\n", dst_id, dst_domain, tag);
    SIP_ADD("To: <sip:%s@%s\r\n", src_id, src_domain);

    char *call_id   = malloc_new_random();
    SIP_ADD("Call-ID: %s\r\n", call_id);
    osip_free(call_id);

    char *cseq;
    osip_cseq_to_str(req->cseq, &cseq);
    SIP_ADD("%s\r\n", cseq);
    osip_free(cseq);

    SIP_ADD("Max-Forwards: 70\r\n");
    SIP_ADD("Content-Type: Application/MANSCDP+xml\r\n");
    SIP_ADD("Content-Length: %lu\r\n", strlen(msg_body));
    SIP_ADD("%s", msg_body);


    /// send response to peer
    int ret = send_data(send_buf, send_len, dst_ip, dst_port);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send message answer failed:%d\n", ret);
        return ret;
    }

    return 0;
}


/*
 * response to message
 */
void CGbExosip::deal_message()
{
    ESipResCode code;
    int type;
    osip_message_t *msg     = recv_msg;

    code = get_msg_type(type);
    if (SIP_RESPONSE_200_OK == code)
    {

        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "deal message, get msg type :%d\n", type);
        code = call_back(type, msg, sizeof(msg), rmt_ip, rmt_port, cur_time);
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "deal message, get msg type failed:%d\n", type);
    }
}

/// get msg body type
ESipResCode CGbExosip::get_msg_type(int &type)
{
    osip_message_t *msg     = recv_msg;
    if ( (NULL == msg->content_length) || (NULL == msg->content_length->value))
    {
        return SIP_RESPONSE_400_BAD_REQ;
    }

    long int len;
    if (false == CStr2Digit::get_lint(msg->content_length->value, len))
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "get content len failed:%d\n", msg->content_length->value);
        return SIP_RESPONSE_485_AMBIGUOUS;
    }

    // len is zero but have content
    if (0 == len)
    {
        return SIP_RESPONSE_400_BAD_REQ;
    }

    osip_content_type_t *content_type   = msg->content_type;
    if ( (NULL == content_type) || (NULL == content_type->type) || (NULL == content_type->subtype) )
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "len:%lld, but content type is null\n", len);
        return SIP_RESPONSE_485_AMBIGUOUS;
    }

    if (strncasecmp(content_type->type, CONTENT_TYPE_APPLICATION, strlen(CONTENT_TYPE_APPLICATION)))
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "unrecognize content type:%n", content_type->type);
        return SIP_RESPONSE_400_BAD_REQ;
    }

    if (!strncasecmp(content_type->subtype, CONTENT_SUBTYPE_MANSCDP_XML, strlen(CONTENT_SUBTYPE_MANSCDP_XML)))
    {
        type = EV_TYPE_MSG_MANSCDPXML;
    }
    else if (!strncasecmp(content_type->subtype, CONTENT_SUBTYPE_SDP, strlen(CONTENT_SUBTYPE_SDP)))
    {
        type = EV_TYPE_MSG_SDP;
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "unrecognize content subtype:%n", content_type->subtype);
        return SIP_RESPONSE_400_BAD_REQ;
    }

    return SIP_RESPONSE_200_OK;
}


// construct function
CGbExosip::CGbExosip(uint16_t p, char *id, char *domain, char *agent, char *ip)
{
#if 0
    local_port          = p;
    recv_msg            = NULL;

    manscdp_sn  = 1;
    snprintf(server_id, sizeof(server_id), "%s", id);
    snprintf(domain_id, sizeof(domain_id), "%s", domain);
    snprintf(user_agent, sizeof(user_agent), "%s", agent);

    snprintf(server_ip, sizeof(server_ip), "%s", ip);
    snprintf(server_port, sizeof(server_port), "%u", local_port);
#endif
}

/*
 * event callback to server
 */
ESipResCode CGbExosip::call_back(int type, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time)
{
    if (NULL != ev_cb)
    {
        if ( (0 == ip) || (0 == port) )
        {
            // ip = uac_ip;
        }
        return ev_cb(ev_cb_context, type, buf, len, rmt_ip, rmt_port, time);
    }

    return SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
}

/*
 * set event callback from sip
 */
void CGbExosip::set_ev_cb(FEvCb cb, void *context)
{
    ev_cb           = cb;
    ev_cb_context   = context;
}


//// parse income data
void CGbExosip::parse_data(void *data, int data_len)
{
    int ret;
    osip_message_t *msg;

    ret = osip_message_init(&msg);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "cannot allocate mem\n");
        return;
    }

    /// parse msg
    ret = osip_message_parse(msg, static_cast<const char *>(data), data_len);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "cannot parse sip message:%s\n", data);
        osip_message_free(msg);
        return;
    }

    if (MSG_IS_REQUEST (msg)) {
        // syntax error
        if (msg->sip_method == NULL || msg->req_uri == NULL) {
            osip_message_free (msg);
            CLog::log(CLog::CLOG_LEVEL_GB_TASK, "sip message syntax error\n");
            return;
        }
    }
    else
    {
        // syntax error
        if (msg->status_code < 100 || msg->status_code > 699) {
            osip_message_free(msg);
            CLog::log(CLog::CLOG_LEVEL_GB_TASK, "sip message syntax error\n");
            return;
        }
    }

    // get ip & port
    recv_msg    = msg;
    get_uac_addr();

    // deal request | response
    if (MSG_IS_REQUEST(msg))
    {
        deal_request();
    }
    else if (MSG_IS_RESPONSE(msg))
    {
        deal_response();
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "unkwon sip message type:%s\n", data);
    }

    // free msg
    osip_message_free(msg);
}

/// data income from link
void CGbExosip::data_in(void *data, int data_len, uint32_t ip, uint16_t port, uint64_t time)
{
    // sip msg must bigger than 32
    if (data_len < 33)
    {
        return;
    }

    rmt_ip      = ip;
    rmt_port    = port;
    cur_time    = time;

    parse_data(data, data_len);
}

char *CGbExosip::malloc_new_random()
{
    char *tmp = (char *) osip_malloc (33);
    unsigned int number = osip_build_random_number();

    if (tmp == NULL)
      return NULL;

    sprintf (tmp, "%u", number);
    return tmp;
}

/* prepare a minimal request (outside of a dialog) with required headers */
/*
 *    method is the type of request. ("INVITE", "REGISTER"...)
 *       to is the remote target URI
 *          transport is either "TCP" or "UDP" (by now, only UDP is implemented!)
 *          */
int CGbExosip::generate_request(osip_message_t ** dest, const char *method, const char *to, const char *from, const char *proxy)
{
#if 0
    /* Section 8.1:
     *      A valid request contains at a minimum "To, From, Call-iD, Cseq,
     *           Max-Forwards and Via
     *              */
    int i;
    osip_message_t *request;
    int doing_register;

    *dest = NULL;

    if (!method || !*method)
      return OSIP_BADPARAMETER;

    i = osip_message_init (&request);
    if (i != 0)
      return i;

    /* prepare the request-line */
    osip_message_set_method (request, osip_strdup (method));
    osip_message_set_version (request, osip_strdup ("SIP/2.0"));
    osip_message_set_status_code (request, 0);
    osip_message_set_reason_phrase (request, NULL);

    doing_register = 0 == strcmp ("REGISTER", method);

    if (doing_register) {
        i = osip_uri_init (&(request->req_uri));
        if (i != 0) {
            osip_message_free (request);
            return i;
        }
        i = osip_uri_parse (request->req_uri, proxy);
        if (i != 0) {
            osip_message_free (request);
            return i;
        }
        i = osip_message_set_to (request, from);
        if (i != 0 || request->to == NULL) {
            if (i >= 0)
              i = OSIP_SYNTAXERROR;
            osip_message_free (request);
            return i;
        }

        /* REMOVE ALL URL PARAMETERS from to->url headers and add them as headers */
        if (request->to != NULL && request->to->url != NULL) {
            osip_uri_t *url = request->to->url;

            while (osip_list_size (&url->url_headers) > 0) {
                osip_uri_header_t *u_header;

                u_header = (osip_uri_param_t *) osip_list_get (&url->url_headers, 0);
                if (u_header == NULL)
                  break;

                if (osip_strcasecmp (u_header->gname, "from") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "to") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "call-id") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "cseq") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "via") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "contact") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "route") == 0) {
                    osip_message_set_route (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "call-info") == 0) {
                    osip_message_set_call_info (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "accept") == 0) {
                    osip_message_set_accept (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "accept-encoding") == 0) {
                    osip_message_set_accept_encoding (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "accept-language") == 0) {
                    osip_message_set_accept_language (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "alert-info") == 0) {
                    osip_message_set_alert_info (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "allow") == 0) {
                    osip_message_set_allow (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "content-type") == 0) {
                    osip_message_set_content_type (request, u_header->gvalue);
                }
                else
                  osip_message_set_header (request, u_header->gname, u_header->gvalue);
                osip_list_remove (&url->url_headers, 0);
                osip_uri_param_free (u_header);
            }
        }
    }
    else {
        /* in any cases except REGISTER: */
        i = osip_message_set_to (request, to);
        if (i != 0 || request->to == NULL) {
            if (i >= 0)
              i = OSIP_SYNTAXERROR;
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: callee address does not seems to be a sipurl: %s\n", to));
            osip_message_free (request);
            return i;
        }

        /* REMOVE ALL URL PARAMETERS from to->url headers and add them as headers */
        if (request->to != NULL && request->to->url != NULL) {
            osip_uri_t *url = request->to->url;

            while (osip_list_size (&url->url_headers) > 0) {
                osip_uri_header_t *u_header;

                u_header = (osip_uri_param_t *) osip_list_get (&url->url_headers, 0);
                if (u_header == NULL)
                  break;

                if (osip_strcasecmp (u_header->gname, "from") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "to") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "call-id") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "cseq") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "via") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "contact") == 0) {
                }
                else if (osip_strcasecmp (u_header->gname, "route") == 0) {
                    osip_message_set_route (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "call-info") == 0) {
                    osip_message_set_call_info (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "accept") == 0) {
                    osip_message_set_accept (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "accept-encoding") == 0) {
                    osip_message_set_accept_encoding (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "accept-language") == 0) {
                    osip_message_set_accept_language (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "alert-info") == 0) {
                    osip_message_set_alert_info (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "allow") == 0) {
                    osip_message_set_allow (request, u_header->gvalue);
                }
                else if (osip_strcasecmp (u_header->gname, "content-type") == 0) {
                    osip_message_set_content_type (request, u_header->gvalue);
                }
                else
                  osip_message_set_header (request, u_header->gname, u_header->gvalue);
                osip_list_remove (&url->url_headers, 0);
                osip_uri_param_free (u_header);
            }
        }

        if (proxy != NULL && proxy[0] != 0) {       /* equal to a pre-existing route set */
            /* if the pre-existing route set contains a "lr" (compliance
             *          with bis-08) then the req_uri should contains the remote target
             *                   URI */
            osip_uri_param_t *lr_param;
            osip_route_t *o_proxy;

            osip_route_init (&o_proxy);
            i = osip_route_parse (o_proxy, proxy);
            if (i != 0) {
                osip_route_free (o_proxy);
                osip_message_free (request);
                return i;
            }

            osip_uri_uparam_get_byname (o_proxy->url, "lr", &lr_param);
            if (lr_param != NULL) {   /* to is the remote target URI in this case! */
                i = osip_uri_clone (request->to->url, &(request->req_uri));
                if (i != 0) {
                    osip_route_free (o_proxy);
                    osip_message_free (request);
                    return i;
                }

                /* "[request] MUST includes a Route header field containing
                 *            the route set values in order." */
                osip_list_add (&request->routes, o_proxy, 0);
            }
            else
              /* if the first URI of route set does not contain "lr", the req_uri
               *            is set to the first uri of route set */
            {
                request->req_uri = o_proxy->url;
                o_proxy->url = NULL;
                osip_route_free (o_proxy);
                /* add the route set */
                /* "The UAC MUST add a route header field containing
                 *            the remainder of the route set values in order.
                 *                       The UAC MUST then place the remote target URI into
                 *                                  the route header field as the last value
                 *                                           */
                osip_message_set_route (request, to);
            }
        }
        else {                      /* No route set (outbound proxy) is used */

            /* The UAC must put the remote target URI (to field) in the req_uri */
            i = osip_uri_clone (request->to->url, &(request->req_uri));
            if (i != 0) {
                osip_message_free (request);
                return i;
            }
        }
    }

    /* set To and From */
    i = osip_message_set_from (request, from);
    if (i != 0 || request->from == NULL) {
        if (i >= 0)
          i = OSIP_SYNTAXERROR;
        osip_message_free (request);
        return i;
    }

    /* REMOVE ALL URL PARAMETERS from from->url headers and add them as headers */
    if (doing_register && request->from != NULL && request->from->url != NULL) {
        osip_uri_t *url = request->from->url;

        while (osip_list_size (&url->url_headers) > 0) {
            osip_uri_header_t *u_header;

            u_header = (osip_uri_param_t *) osip_list_get (&url->url_headers, 0);
            if (u_header == NULL)
              break;

            osip_list_remove (&url->url_headers, 0);
            osip_uri_param_free (u_header);
        }
    }

    if (request->to != NULL && request->to->url != NULL) {
        osip_list_iterator_t it;
        osip_uri_param_t* u_param = (osip_uri_param_t*)osip_list_get_first(&request->to->url->url_params, &it);

        while (u_param != NULL) {
            if (u_param->gvalue != NULL && u_param->gname!=NULL && osip_strcasecmp (u_param->gname, "method") == 0) {
                osip_list_iterator_remove(&it);
                osip_uri_param_free (u_param);
                break;
            }
            u_param = (osip_uri_param_t *)osip_list_get_next(&it);
        }
    }

    if (request->from != NULL && request->from->url != NULL) {
        osip_list_iterator_t it;
        osip_uri_param_t* u_param = (osip_uri_param_t*)osip_list_get_first(&request->from->url->url_params, &it);

        while (u_param != NULL) {
            if (u_param->gvalue != NULL && u_param->gname!=NULL && osip_strcasecmp (u_param->gname, "method") == 0) {
                osip_list_iterator_remove(&it);
                osip_uri_param_free (u_param);
                break;
            }
            u_param = (osip_uri_param_t *)osip_list_get_next(&it);
        }
    }

    if (request->req_uri) {
        osip_list_iterator_t it;
        osip_uri_param_t* u_param = (osip_uri_param_t*)osip_list_get_first(&request->req_uri->url_params, &it);

        while (u_param != NULL) {
            if (u_param->gvalue != NULL && u_param->gname!=NULL && osip_strcasecmp (u_param->gname, "method") == 0) {
                osip_list_iterator_remove(&it);
                osip_uri_param_free (u_param);
                break;
            }
            u_param = (osip_uri_param_t *)osip_list_get_next(&it);
        }
    }

    /* add a tag */
    osip_from_set_tag (request->from, malloc_new_random ());

    /* set the cseq and call_id header */
    {
        osip_call_id_t *callid;
        osip_cseq_t *cseq;
        char *num;
        char *cidrand;

        /* call-id is always the same for REGISTRATIONS */
        i = osip_call_id_init (&callid);
        if (i != 0) {
            osip_message_free (request);
            return i;
        }
        cidrand = malloc_new_random ();
        osip_call_id_set_number (callid, cidrand);

        request->call_id = callid;

        i = osip_cseq_init (&cseq);
        if (i != 0) {
            osip_message_free (request);
            return i;
        }
        num = osip_strdup (doing_register ? "1" : "20");
        osip_cseq_set_number (cseq, num);
        osip_cseq_set_method (cseq, osip_strdup (method));
        request->cseq = cseq;

        if (cseq->method == NULL || cseq->number == NULL) {
            osip_message_free (request);
            return OSIP_NOMEM;
        }
    }

    /* special values to be replaced in transport layer (eXtl_*.c files) */
    snprintf(temp_buf, 200, "SIP/2.0/UDP %s:%s;branch=z9hG4bK%u", server_ip, server_port, osip_build_random_number());
    osip_message_set_via (request, temp_buf);

    /* always add the Max-Forward header */
    osip_message_set_max_forwards (request, "70");        /* a UA should start a request with 70 */

    if (0 == strcmp ("REGISTER", method)) {
    }
    else if (0 == strcmp ("INFO", method)) {
    }
    else if (0 == strcmp ("OPTIONS", method)) {
        osip_message_set_accept (request, "application/sdp");
    }

    osip_message_set_user_agent (request, user_agent);
    /*  else if ... */
    *dest = request;
#endif
    return OSIP_SUCCESS;
}

/// get rmt TCP/IP addr
void CGbExosip::get_uac_addr()
{
    int ret;
    osip_contact_t *co_msg = NULL;

    ret = osip_message_get_contact(recv_msg, 0, &co_msg);
    if ((ret < 0 || co_msg == NULL || co_msg->url == NULL) || (NULL == co_msg->url->port) || (NULL == co_msg->url->host))
    {
        unsigned char *bytes = (unsigned char *)&rmt_ip;
        snprintf(uac_ip, sizeof(uac_ip), "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3]);
        snprintf(uac_port, sizeof(uac_port), "%u", ntohs(rmt_port));
        return;
    }

    snprintf(uac_ip, sizeof(uac_ip), "%s", co_msg->url->host);
    snprintf(uac_port, sizeof(uac_port), "%s", co_msg->url->port);
}

/// send data to peer
int CGbExosip::send_data(osip_message_t *to_send_msg)
{
    int ret;
    size_t          length;
    char            *msg_str    = NULL;
    ret = osip_message_to_str(to_send_msg, &msg_str, &length);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "register msg to str failed:%d\n", ret);
    }

    ret = snd_buf_cb(snd_buf_cb_ctx, msg_str, (int)length, rmt_ip, rmt_port, cur_time);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send message answer failed:%d\n", ret);
    }
    osip_free(msg_str);

    return ret;
}

int CGbExosip::send_data(char *msg, uint32_t len, uint32_t ip, uint16_t port)
{
    int ret = snd_buf_cb(snd_buf_cb_ctx, msg, (size_t)len, ip, port, cur_time);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send data failed:%d\n", ret);
    }

    return ret;
}


/// send data to peer
int CGbExosip::send_data(osip_message_t *to_send_msg, uint32_t ip, uint16_t port)
{
    int ret;
    size_t          length;
    char            *msg_str    = NULL;
    ret = osip_message_to_str(to_send_msg, &msg_str, &length);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "register msg to str failed:%d\n", ret);
    }

    ret = snd_buf_cb(snd_buf_cb_ctx, msg_str, (int)length, ip, port, cur_time);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send data failed:%d\n", ret);
    }
    osip_free(msg_str);

    return ret;
}


int CGbExosip::query_record_info(char *dst_id, char *dst_domain, char *src_id, char *src_domain, uint32_t src_ip, uint16_t src_port,
                                    uint32_t dst_ip, uint16_t dst_port, uint32_t seq, uint32_t manscdp_sn, GB::CGbRecordQueryParam &query_param)
{
    int ret;
    send_len = 0;
    unsigned char *bytes = (unsigned char *)&src_ip;

    /// Message 目的域名或目的IP
    SIP_ADD("MESSAGE sip:%s@%s SIP/2.0\r\n", dst_id, dst_domain);

    /// To 目的设备@目的域名
    SIP_ADD("To: <sip:%s@%s>\r\n", dst_id, dst_domain);

    /// From 源设备编码@源域???
    char *tag = malloc_new_random();
    if (!tag)
    {
        return GB_ERR_NOMEM;
    }
    SIP_ADD("From: <sip:%s@%s>;tag=%s\r\n", src_id, src_domain, tag);
    osip_free(tag);

    /// Cseq
    SIP_ADD("CSeq: %u MESSAGE\r\n", seq);

    /// CALL-ID
    tag = malloc_new_random();
    if (!tag)
    {
        return GB_ERR_NOMEM;
    }
    SIP_ADD("Call-ID: %s\r\n", tag);
    osip_free(tag);

    /// Via 源域名或源IP
    SIP_ADD("Via: SIP/2.0/UDP %u.%u.%u.%u:%u;branch=z9hG4bK%u\r\n", bytes[0], bytes[1], bytes[2], bytes[3], dst_port, osip_build_random_number());

    char start_tm[32] = { 0 }, end_tm[32] = { 0 };
    struct tm v_time1, v_time2;
#ifdef _MSC_VER
    localtime_s(&v_time1, (time_t *)&query_param.start_time);
    localtime_s(&v_time2, (time_t *)&query_param.end_time);
#else
    localtime_r(&query_param.start_time, &v_time1);
    localtime_r(&query_param.end_time, &v_time2);
#endif
    snprintf(start_tm, sizeof(start_tm), "%04d-%02d-%02dT%02d:%02d:%02d", v_time1.tm_year + 1900, v_time1.tm_mon + 1,
        v_time1.tm_mday, v_time1.tm_hour, v_time1.tm_min, v_time1.tm_sec);
    snprintf(end_tm, sizeof(end_tm), "%04d-%02d-%02dT%02d:%02d:%02d", v_time2.tm_year + 1900, v_time2.tm_mon + 1,
        v_time2.tm_mday, v_time2.tm_hour, v_time2.tm_min, v_time2.tm_sec);


    char tmp_buf[256];
    snprintf(tmp_buf, sizeof(tmp_buf), "<StartTime>%s</StartTime>\r\n<EndTime>%s</EndTime>\r\n", start_tm, end_tm);
    snprintf(tmp_buf + strlen(tmp_buf), sizeof(tmp_buf) - strlen(tmp_buf), "<FilePath>%s</FilePath>\r\n"
        "<Address>Address 1</Address>\r\n<Secrecy>0</Secrecy>\r\n<Type>time</Type>\r\n"
        "<RecorderID>%s</RecorderID>\r\n",
        query_param.dev_id, query_param.dev_id);

    char content[512];
    snprintf(content, sizeof(content), "%s%d%s%s%s%s%s",
        "<?xml version=\"1.0\"?>\r\n\r\n"
        "<Query>\r\n"
        "<CmdType>RecordInfo</CmdType>\r\n"
        "<SN>", manscdp_sn,
        "</SN>\r\n"
        "<DeviceID>", query_param.dev_id,
        "</DeviceID>\r\n", tmp_buf,
        "</Query>\r\n");






    SIP_ADD("Max-Forwards: 70\r\nContent-Type: application/MANSCDP+xml\r\nContent-Length: %lu\r\n\r\n", strlen(content));

    SIP_ADD("%s", content);

    // query device catalog
    ret = send_data(send_buf, send_len, dst_ip, dst_port);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send Query::record info request failed,dev:%s, ip:%x,port:%u\n",
            dst_id, dst_ip, dst_port);
    }

    return GB_SUCCESS;
}


/// 查询目录结构
int CGbExosip::query_catalog(char *dst_id, char *dst_domain, char *src_id, char *src_domain, uint32_t src_ip, uint16_t src_port,
                                uint32_t dst_ip, uint16_t dst_port, uint32_t seq, uint32_t manscdp_sn)
{
    int ret;
    send_len    = 0;
    unsigned char *bytes = (unsigned char *)&src_ip;

    /// Message 目的域名或目的IP
    SIP_ADD("MESSAGE sip:%s@%s SIP/2.0\r\n", dst_id, dst_domain);

    /// To 目的设备@目的域名
    SIP_ADD("To: <sip:%s@%s>\r\n", dst_id, dst_domain);

    /// From 源设备编码@源域???

    char *tag   = malloc_new_random();
    if (!tag)
    {
        return GB_ERR_NOMEM;
    }
    SIP_ADD("From: <sip:%s@%s>;tag=%s\r\n", src_id, src_domain, tag);
    osip_free(tag);

    /// Cseq
    SIP_ADD("CSeq: %u MESSAGE\r\n", seq);

    /// CALL-ID
    tag   = malloc_new_random();
    if (!tag)
    {
        return GB_ERR_NOMEM;
    }
    SIP_ADD("Call-ID: %s\r\n", tag);
    osip_free(tag);

    /// Via 源域名或源IP
    SIP_ADD("Via: SIP/2.0/UDP %u.%u.%u.%u:%u;branch=z9hG4bK%u\r\n", bytes[0], bytes[1], bytes[2], bytes[3], dst_port, osip_build_random_number());

    char content[256];
    snprintf(content, sizeof(content), "%s%d%s%s%s",
    "<?xml version=\"1.0\"?>\r\n\r\n"
    "<Query>\r\n"
    "<CmdType>Catalog</CmdType>\r\n"
    "<SN>", manscdp_sn,
    "</SN>\r\n"
    "<DeviceID>", dst_id,
    "</DeviceID>\r\n"
    "</Query>\r\n");

    SIP_ADD("Max-Forwards: 70\r\nContent-Type: application/MANSCDP+xml\r\nContent-Length: %lu\r\n\r\n", strlen(content));

    SIP_ADD("%s", content);

    // query device catalog
    ret = send_data(send_buf, send_len, dst_ip, dst_port);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send Query::catalog request failed,dev:%s, ip:%x,port:%u\n",
                    dst_id, dst_ip, dst_port);
    }

    return ret;
}


// 注册
int CGbExosip::reg_to_server(char *server_id, char *server_domain, char *src_id, char *src_domain,
                                uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                                uint32_t seq, char *auth_head, char *from_tag, char *call_id)
{
    int ret;
    unsigned char *bytes = (unsigned char *)&src_ip;
    send_len    = 0;

    SIP_ADD("REGISTER sip:%s@%s SIP/2.0\r\n", server_id, server_domain);
    SIP_ADD("Via: SIP/2.0/UDP %u.%u.%u.%u:%u;rport;branch=z9hG4bK%u\r\n",
                bytes[0], bytes[1], bytes[2], bytes[3], src_port, osip_build_random_number());

    char *tag;
    if (from_tag)
    {
        tag = from_tag;
    }
    else
    {
        tag   = malloc_new_random();
    }
    SIP_ADD("From: <sip:%s@%s>;tag=%s\r\n", server_id, server_domain, tag);
    if (!from_tag)
    {
        osip_free(tag);
    }

    SIP_ADD("To: <sip:%s@%8s>\r\n", server_id, server_domain);

    if (call_id)
    {
        SIP_ADD("Call-ID: %s\r\n", call_id);
    }
    else
    {
        tag   = malloc_new_random();
        SIP_ADD("Call-ID: %s\r\n", tag);
        osip_free(tag);
    }

    SIP_ADD("CSeq: %u REGISTER\r\n", seq);

    SIP_ADD("Contact: <sip:%s@%u.%u.%u.%u:%u>\r\n", server_id, bytes[0], bytes[1], bytes[2], bytes[3], src_port);
    if (auth_head)
    {
        SIP_ADD("%s", auth_head);
    }

    SIP_ADD("Max-Forwards: 70\r\n");
    SIP_ADD("User-Agent: IP Camera\r\n");
    SIP_ADD("Expires: 3600\r\n");
    SIP_ADD("Content-Length: 0\r\n\r\n");

    // query device catalog
    ret = send_data(send_buf, send_len, dst_ip, dst_port);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send register request failed,dev:%s, ip:%x,port:%u\n",
                    server_id, dst_ip, dst_port);
    }

    return ret;
}

void CGbExosip::get_rmt_deviceid(char *device_id_buf, int device_id_buf_len)
{
    snprintf(device_id_buf, device_id_buf_len, "%s", recv_msg->req_uri->username);
}


/// 璁剧疆鍙???佸洖???
void CGbExosip::set_send_data_cb(FSendBufCb buf_cb, void *buf_ctx, FSendMsgCb msg_cb, void *msg_ctx)
{
    snd_buf_cb      = buf_cb;
    snd_buf_cb_ctx  = buf_ctx;
    snd_msg_cb      = msg_cb;
    snd_msg_cb_ctx  = msg_ctx;
}
