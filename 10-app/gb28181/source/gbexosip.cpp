#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h> 
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>


// prj include
#include "common.h"
#include "gb.h"
#include "gbexosip.h"
#include "reg.h"
#include "gblink.h"
using namespace GB;

#define GBSIP_VER    "SIP/2.0" 

#define GBSIP_METHOD_REGISTER   "REGISTER"
#define GBSIP_METHOD_MESSAGE    "MESSAGE"
#define GBSIP_METHOD_INVITE     "INVITE"
#define GBSIP_METHOD_ACK        "ACK"
#define GBSIP_METHOD_BYE        "BYE"


static char* gb_record_info_type_list[] = 
{
    "",
    "time",
    "alarm",
    "manual",
    "all"
};


const char *CGbExosip::AUTH_TYPE                    = "Digest";
const char *CGbExosip::AUTH_ALGORITHM               = "MD5";

const char *CGbExosip::CONTENT_TYPE_APPLICATION     = "application";
const char *CGbExosip::CONTENT_SUBTYPE_MANSCDP_XML  = "MANSCDP+xml";
const char *CGbExosip::CONTENT_SUBTYPE_SDP          = "sdp";

int CGbExosip::start()
{
    return GB_SUCCESS;
}

// run task
void *CGbExosip::routine(void *arg)
{
    return NULL;
}

// run server
void CGbExosip::run()
{
}

/// query rmt cata_log
int CGbExosip::query_record_info(char *dst_dev_id, char *dst_catalog_id, char *start_time, char *end_time, int sn, uint32_t ip, uint16_t port)
{
    int ret;
    osip_message_t *dest;
    char domain[8];
    char to[256];   //     = "<sip:34020000001180000012@172.16.1.235:5060>";

    snprintf(domain, 8, "%s", dst_dev_id);
    snprintf(to, sizeof(to), "<sip:%s@%s>", dst_dev_id, uac_ip);
    snprintf(temp_buf, sizeof(temp_buf), "<sip:%s@%s>", server_id, domain_id);
    ret = generate_request(&dest, GBSIP_METHOD_MESSAGE, to, temp_buf, NULL);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "register msg to str failed:%d\n", ret);
        return -1;
    }

    if (NULL == dest->req_uri->port)
    {
        osip_uri_set_port(dest->req_uri, osip_strdup(uac_port));
    }

    snprintf(to, sizeof(to), "<StartTime>%s</StartTime>\r\n<EndTime>%s</EndTime>\r\n", start_time, end_time);
    snprintf(to + strlen(to), sizeof(to) - strlen(to), "<FilePath>%s</FilePath>\r\n"
                "<Address>Address 1</Address>\r\n<Secrecy>0</Secrecy>\r\n<Type>time</Type>\r\n"
                "<RecorderID>%s</RecorderID>\r\n",
                dst_catalog_id, dst_catalog_id);

    snprintf(temp_buf, sizeof(temp_buf), "%s%d%s%s%s%s%s", 
    "<?xml version=\"1.0\"?>\r\n\r\n"
    "<Query>\r\n" 
    "<CmdType>RecordInfo</CmdType>\r\n"
    "<SN>", sn++, 
    "</SN>\r\n" 
    "<DeviceID>", dst_catalog_id,
    "</DeviceID>\r\n", to, 
    "</Query>\r\n");
    osip_message_set_body(dest, temp_buf, strlen (temp_buf));

    snprintf(temp_buf, sizeof(temp_buf), "%s/%s", CONTENT_TYPE_APPLICATION, CONTENT_SUBTYPE_MANSCDP_XML);
    osip_message_set_content_type(dest, temp_buf);

    // query device catalog
    ret = send_data(dest, ip, port);
    osip_message_free(dest);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send register answer failed:%d\n", ret);
    }

    return ret;
}

/// query rmt cata_log
int CGbExosip::query_catalog(char *dst_dev_id, uint32_t ip, uint16_t port)
{
    int ret;
    osip_message_t *dest;
    char domain[8];
    char to[128];   //     = "<sip:34020000001180000012@172.16.1.235:5060>";

    snprintf(domain, 8, "%s", dst_dev_id);
    // snprintf(to, sizeof(to), "<sip:%s@%s>", dst_dev_id, domain);
    snprintf(to, sizeof(to), "<sip:%s@%s>", dst_dev_id, uac_ip);
    snprintf(temp_buf, sizeof(temp_buf), "<sip:%s@%s>", server_id, domain_id);
    ret = generate_request(&dest, GBSIP_METHOD_MESSAGE, to, temp_buf, NULL);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "register msg to str failed:%d\n", ret);
        return -1;
    }

    if (NULL == dest->req_uri->port)
    {
        osip_uri_set_port(dest->req_uri, osip_strdup(uac_port));
    }

    snprintf(temp_buf, sizeof(temp_buf), "%s%d%s%s%s", 
    "<?xml version=\"1.0\"?>\r\n\r\n"
    "<Query>\r\n" 
    "<CmdType>Catalog</CmdType>\r\n"
    "<SN>", manscdp_sn++, 
    "</SN>\r\n" 
    "<DeviceID>", dst_dev_id,
    "</DeviceID>\r\n"
    "</Query>\r\n");
    osip_message_set_body(dest, temp_buf, strlen (temp_buf));

    snprintf(temp_buf, sizeof(temp_buf), "%s/%s", CONTENT_TYPE_APPLICATION, CONTENT_SUBTYPE_MANSCDP_XML);
    osip_message_set_content_type(dest, temp_buf);

    // query device catalog
    ret = send_data(dest, ip, port);
    osip_message_free(dest);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send register answer failed:%d\n", ret);
    }

    return ret;
}

void CGbExosip::deal_register()
{
    ESipResCode code;
    int ret;
    osip_message_t *msg     = recv_msg; // exosip_evt->request; 
    osip_message_t *answer  = NULL;

    // check if authorization info right
    code = SIP_RESPONSE_401_UNAUTHORIZED;
    if (msg->authorizations.nb_elt && msg->authorizations.node)
    {
        code = call_back(EV_TYPE_AUTH_REG, msg, sizeof(msg), rmt_ip, rmt_port, cur_time);
    }
    else
    {
        code = call_back(EV_TYPE_UNAUTH_REG, msg, sizeof(msg), rmt_ip, rmt_port, cur_time);
    }

    ret = build_response(&answer, code, msg);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "build answer failed:%d\n", ret);
    }

    if (SIP_RESPONSE_401_UNAUTHORIZED == code)
    {
        CRandom::rand();
        snprintf(nonce, sizeof(nonce), "%x%x%x%x%x",
                    CRandom::rand(), CRandom::rand(), CRandom::rand(), CRandom::rand(), CRandom::rand());
        snprintf(temp_buf, sizeof(temp_buf), "Digest realm=\"%s\",nonce=\"%s\",algorithm=MD5\r\n",
                    server_id, nonce);

        ret = osip_message_set_www_authenticate(answer, temp_buf);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_GB_TASK, "set authorization failed:%d\n", ret);
        }
    }

    // send response 
    ret = send_data(answer);
    osip_message_free(answer);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send register answer failed:%d\n", ret);
    }
}

// deal message
void CGbExosip::deal_response()
{
    int             ret;
    osip_via_t      *via_tmp;
    char *from, *to, *call_id, *cseq, *via;
    osip_message_t *msg = recv_msg;
    char            snd_buf[1024];
    int             snd_buf_len;

    if (NULL == msg->cseq->method)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "recv response msg, withno method\n");
        return;
    }
    else if (!strncasecmp(msg->cseq->method, GBSIP_METHOD_MESSAGE, strlen(GBSIP_METHOD_MESSAGE)))
    {
        return;
    }
    else if (!strncasecmp(msg->cseq->method, GBSIP_METHOD_INVITE, strlen(GBSIP_METHOD_INVITE)))
    {
        // to response
        ret = osip_message_get_via (msg, 0, &via_tmp);
        if (0 != ret)
        {
        }
        osip_call_id_to_str(msg->call_id, &call_id);
        osip_to_to_str(msg->to, &to);
        osip_from_to_str(msg->from, &from);
        osip_cseq_to_str(msg->cseq, &cseq);
        osip_via_to_str(via_tmp, &via);

        // status
        snprintf(snd_buf, sizeof(snd_buf), "%s %s@%s:%s %s\r\n",
                    "ACK", msg->to->url->username, msg->to->url->host, msg->to->url->port, GBSIP_VER);
        snd_buf_len = strlen(snd_buf);

        // from/call_id/to/cseq/via 
        snprintf(snd_buf + snd_buf_len, sizeof(snd_buf) - snd_buf_len, "Via: %s\r\nCall-ID: %s\r\nFrom: %s\r\nTo: %s\r\nCSeq: %s %s\r\n",
                    via, call_id, from, to, msg->cseq->number, "ACK");
        snd_buf_len = strlen(snd_buf);

        snprintf(snd_buf + snd_buf_len, sizeof(snd_buf) - snd_buf_len, "Contact: <sip:%s@%s:%s>\r\n",
                    server_id, server_ip, server_port);
        snd_buf_len = strlen(snd_buf);

        snprintf(snd_buf + snd_buf_len, sizeof(snd_buf) - snd_buf_len, "User-Agent: %s\r\n", user_agent);
        snd_buf_len = strlen(snd_buf);

        // set Content-Length
        snprintf(snd_buf + snd_buf_len, sizeof(snd_buf) - snd_buf_len, "Content-Length: 0\r\n\r\n");
        snd_buf_len = strlen(snd_buf);

        ret = snd_buf_cb(snd_buf_cb_ctx, snd_buf, snd_buf_len, rmt_ip, rmt_port, cur_time);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send message answer failed:%d\n", ret);
        }

        /// free resource
        osip_free(call_id);
        osip_free(to);
        osip_free(from);
        osip_free(cseq);
        osip_free(via);
    }
}

// deal message
void CGbExosip::deal_request()
{
    osip_message_t *msg = recv_msg; 

    if (msg->call_id == NULL || msg->from == NULL || msg->to == NULL)
    {
        return;
    }

    if (!strncasecmp(msg->sip_method, GBSIP_METHOD_REGISTER, strlen(GBSIP_METHOD_REGISTER))) 
    {
        deal_register();
    }
    else if (!strncasecmp(msg->sip_method, GBSIP_METHOD_MESSAGE, strlen(GBSIP_METHOD_MESSAGE)))
    {
        deal_message();
    }
    else if (!strncasecmp(msg->sip_method, GBSIP_METHOD_INVITE, strlen(GBSIP_METHOD_INVITE)))
    {
        deal_invite();
    }
    else if (!strncasecmp(msg->sip_method, GBSIP_METHOD_ACK, strlen(GBSIP_METHOD_ACK)))
    {
        deal_ack();
    }
    else if (!strncasecmp(msg->sip_method, GBSIP_METHOD_BYE, strlen(GBSIP_METHOD_BYE)))
    {
        deal_bye();
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "GBstack invalid cmd:%s\n", msg->sip_method);
        /// gbsip_reply_error(c, SIP_RESPONSE_400_BAD_REQ);
    }
}

void CGbExosip::deal_bye()
{
    ESipResCode code;
    int ret;
    osip_message_t *answer  = NULL;
    osip_message_t *msg     = recv_msg;


    /// send response info to remote
    code = call_back(EV_TYPE_REQ_BYE, recv_msg, sizeof(recv_msg));

    /// 返回错误
    ret = build_response(&answer, code, msg);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "bye response build failed:%d\n", ret);
        return;
    }

    // response to peer
    ret = send_data(answer);
    osip_message_free(answer);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send bye response answer failed:%d\n", ret);
    }
}

/*
 * deal income ACK request 
 */
void CGbExosip::deal_ack()
{
    /// send ACK info to msgcenter and forward media-data
    call_back(EV_TYPE_REQ_ACK, recv_msg, sizeof(recv_msg));
}

/*
 * deal income INVITE Req
 */
void CGbExosip::deal_invite()
{
    ESipResCode code;
    int ret;
    osip_message_t *answer  = NULL;
    osip_message_t *msg     = recv_msg;

    /// send invite info to msgcenter and wait for reply
    code = call_back(EV_TYPE_REQ_INVITE, msg, sizeof(msg));

    if (SIP_RESPONSE_200_OK == code)
    {
        return;
    }

    /// 返回错误
    ret = build_response(&answer, code, msg);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "message invite build failed:%d\n", ret);
        return;
    }

    // response to peer
    ret = send_data(answer);
    osip_message_free(answer);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send invite answer failed:%d\n", ret);
    }
}

/*
 * response to message
 */
void CGbExosip::deal_message()
{
    ESipResCode code;
    int ret, type;
    osip_message_t *answer  = NULL;
    osip_message_t *msg     = recv_msg;

    code = get_msg_type(type);
    if (SIP_RESPONSE_200_OK == code)
    {
        code = call_back(type, msg, sizeof(msg), rmt_ip, rmt_port, cur_time);
    }

    /// response
    ret = build_response(&answer, code, msg);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "message answer build failed:%d\n", ret);
        return;
    }

    /// send response to peer
    ret = send_data(answer);
    osip_message_free(answer);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send message answer failed:%d\n", ret);
    }
}

/// 获取msg消息类型,主要是解析body类型
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
        type = EV_TYPE_DATA_SDP;
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "unrecognize content subtype:%n", content_type->subtype);
        return SIP_RESPONSE_400_BAD_REQ;
    }

    return SIP_RESPONSE_200_OK;
}


/*
 * get body type and content
 */
ESipResCode CGbExosip::parse_body()
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

    int type;
    if (!strncasecmp(content_type->subtype, CONTENT_SUBTYPE_MANSCDP_XML, strlen(CONTENT_SUBTYPE_MANSCDP_XML)))
    {
        type = EV_TYPE_DATA_MANSCDPXML;
    }
    else if (!strncasecmp(content_type->subtype, CONTENT_SUBTYPE_SDP, strlen(CONTENT_SUBTYPE_SDP)))
    {
        type = EV_TYPE_DATA_SDP;
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "unrecognize content subtype:%n", content_type->subtype);
        return SIP_RESPONSE_400_BAD_REQ;
    }

    osip_body_t *body_tmp = NULL;
    char        *body = NULL;
    size_t       body_len;
    if ( (0 != osip_message_get_body(msg, 0, &body_tmp)) || (NULL == body_tmp) )
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip get body from msg failed\n");
        return SIP_RESPONSE_485_AMBIGUOUS;
    }

    if ( (0 != osip_body_to_str(body_tmp, &body, &body_len)) || (NULL == body) )
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip body to str failed\n");
        return SIP_RESPONSE_485_AMBIGUOUS;
    }

    ESipResCode ret_code;
    ret_code = call_back(type, body, len);

    osip_free(body);

    return ret_code;
}

// construct function
CGbExosip::CGbExosip(uint16_t p, char *id, char *domain, char *agent, char *ip)
{
    port            = p;

    recv_msg        = NULL;

    manscdp_sn  = 1;
    snprintf(server_id, sizeof(server_id), "%s", id);
    snprintf(domain_id, sizeof(domain_id), "%s", domain);
    snprintf(user_agent, sizeof(user_agent), "%s", agent);

    snprintf(server_ip, sizeof(server_ip), "%s", ip);
    snprintf(server_port, sizeof(server_port), "%u", port);
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

int CGbExosip::build_response(osip_message_t **dest, int status, osip_message_t *request)
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

    osip_message_set_user_agent (response, user_agent);

    *dest = response;
    return OSIP_SUCCESS;
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

int CGbExosip::invite(MN::CMediaInvite &invite_param)
{
    int ret;
    osip_message_t *dest;
    unsigned char *bytes = (unsigned char *)&invite_param.dev_ip;
    char to[128];   //     = "<sip:34020000001180000012@172.16.1.235:5060>";

    snprintf(to, sizeof(to), "<sip:%s@%u.%u.%u.%u:%u>", invite_param.device_id,
                bytes[0], bytes[1], bytes[2], bytes[3], ntohs(invite_param.dev_port));
    snprintf(temp_buf, sizeof(temp_buf), "<sip:%s@%s>", server_id, domain_id);
    ret = generate_request(&dest, GBSIP_METHOD_INVITE, to, temp_buf, NULL);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "register msg to str failed:%d\n", ret);
        return -1;
    }

    char *psz_sn   = malloc_new_random();
    char *psz_sn1  = malloc_new_random();
    snprintf(temp_buf, sizeof(temp_buf), "%s:0%s,%s:0%s\r\n",
                invite_param.device_id, psz_sn, server_id, psz_sn1);
    osip_generic_param_add (&dest->headers, osip_strdup ("Subject"), osip_strdup (temp_buf));
    osip_free(psz_sn);
    osip_free(psz_sn1);

    snprintf(temp_buf, sizeof(temp_buf), "v=0\r\no=%s 0 0 IN IP4 %s\r\n",
                server_id, server_ip);
    temp_len = strlen(temp_buf);
        
    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "s=Play\r\nc=IN IP4 %s\r\nt=0 0\r\n", server_ip);
    temp_len = strlen(temp_buf);
        
    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "m=video %u RTP/AVP 96\r\ni=primary\r\na=recvonly\r\n",
                invite_param.local_net_info.rtp_port);
    temp_len = strlen(temp_buf);
        
    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "a=rtpmap:96 PS/90000\r\ny=0%u\r\n", invite_param.ssrc);
    temp_len = strlen(temp_buf);

    osip_message_set_body(dest, temp_buf, temp_len);

    snprintf(temp_buf, sizeof(temp_buf), "%s/%s", CONTENT_TYPE_APPLICATION, CONTENT_SUBTYPE_SDP);
    osip_message_set_content_type(dest, temp_buf);

    // query device catalog
    ret = send_data(dest, invite_param.dev_ip, invite_param.dev_port);
    osip_message_free(dest);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send invite request failed,dev:%s, ip:%x,port:%u\n", 
                    invite_param.device_id, invite_param.dev_ip, ntohs(invite_param.dev_port));
    }

    return ret;
}

void CGbExosip::get_rmt_deviceid(char *device_id_buf, int device_id_buf_len)
{
    snprintf(device_id_buf, device_id_buf_len, "%s", recv_msg->req_uri->username);
}

int CGbExosip::reponse_record_info(MN::CMsgGbResRecordInfo &gb_rec_res_info, MN::CMsgGbQueryRecordInfo gb_rec_req_info,
                                    void *item_buf, int item_buf_len)
{
    int ret;
    osip_message_t *dest;
    unsigned char *bytes = (unsigned char *)&gb_rec_req_info.rmt_ip;
    char to[128];
    uint16_t record_item_num = item_buf_len / sizeof(MN::CMsgGbRecordItem);
    record_item_num = CBBMIN(record_item_num, gb_rec_res_info.list_num);

    snprintf(to, sizeof(to), "<sip:%s@%u.%u.%u.%u:%u>", gb_rec_req_info.device_id,
                bytes[0], bytes[1], bytes[2], bytes[3], ntohs(gb_rec_req_info.rmt_port));
    snprintf(temp_buf, sizeof(temp_buf), "<sip:%s@%s>", server_id, domain_id);
    ret = generate_request(&dest, GBSIP_METHOD_MESSAGE, to, temp_buf, NULL);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "response catalog msg to str failed:%d\n", ret);
        return -1;
    }

    snprintf(temp_buf, sizeof(temp_buf), "%s/%s", CONTENT_TYPE_APPLICATION, CONTENT_SUBTYPE_MANSCDP_XML);
    osip_message_set_content_type(dest, temp_buf);

    snprintf(temp_buf, sizeof(temp_buf), "<?xml version=\"1.0\" encoding=\"gb2312\"?>\r\n<Response>\n<CmdType>RecordInfo</CmdType>\n");
    temp_len = strlen(temp_buf);
       
    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<SN>%u</SN>\n<DeviceID>%s</DeviceID>\n<Name>%s</Name>\n",
                (int)gb_rec_req_info.ctx, gb_rec_req_info.device_id, gb_rec_res_info.name);
    temp_len = strlen(temp_buf);

    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<SumNum>%u</SumNum>\n<RecordList Num=\"%u\">\n",
                gb_rec_res_info.sum_num, record_item_num);
    temp_len = strlen(temp_buf);
           
    if (record_item_num > 0)
    {
        char start_tm[32] = {0}, end_tm[32] = {0};
        struct tm v_time1, v_time2;
        for(int i = 0; i < record_item_num; i++)
        {
            MN::CMsgGbRecordItem gb_rec_item;
            memcpy(&gb_rec_item, (uint8_t *)item_buf + i * sizeof(gb_rec_item), sizeof(gb_rec_item));

            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<Item>\n<DeviceID>%s</DeviceID>\n<Name>%s</Name>\n",
                        gb_rec_item.device_id, gb_rec_item.name);
            temp_len = strlen(temp_buf);
        
            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<FilePath>%s</FilePath>\n<Address>%s</Address>\n",
                        gb_rec_item.file_path, gb_rec_item.address);
            temp_len = strlen(temp_buf);


            localtime_r(&gb_rec_item.start_time, &v_time1);
            localtime_r(&gb_rec_item.end_time, &v_time2);
            snprintf(start_tm, sizeof(start_tm), "%04d-%02d-%02dT%02d:%02d:%02d", v_time1.tm_year + 1900, v_time1.tm_mon + 1,
                        v_time1.tm_mday, v_time1.tm_hour, v_time1.tm_min, v_time1.tm_sec);
            snprintf(end_tm, sizeof(end_tm), "%04d-%02d-%02dT%02d:%02d:%02d", v_time2.tm_year + 1900, v_time2.tm_mon + 1,
                        v_time2.tm_mday, v_time2.tm_hour, v_time2.tm_min, v_time2.tm_sec);
            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<StartTime>%s</StartTime>\n<EndTime>%s</EndTime>\n",
                        start_tm, end_tm);
            temp_len = strlen(temp_buf);


            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len,
                        "<Secrecy>%u</Secrecy>\n<Type>%s</Type>\n</Item>\n",
                        gb_rec_item.secrecy, gb_record_info_type_list[(CBBMIN(gb_rec_item.type, 4))]);
            temp_len = strlen(temp_buf);
        }    
    }

    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "</RecordList>\n</Response>\n");
    temp_len = strlen(temp_buf);
           
    osip_message_set_body(dest, temp_buf, temp_len);

    // query device catalog
    ret = send_data(dest, gb_rec_req_info.rmt_ip, gb_rec_req_info.rmt_port);
    osip_message_free(dest);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send catalog response failed:%d\n", ret);
    }

    return ret;
}

int CGbExosip::reponse_catalog(MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len)
{
    int ret;
    osip_message_t *dest;
    unsigned char *bytes = (unsigned char *)&query_catalog_param.rmt_ip;
    char to[128];
    uint16_t catalog_num = catalog_buf_len / sizeof(MN::CCatalogItem);

    snprintf(to, sizeof(to), "<sip:%s@%u.%u.%u.%u:%u>", query_catalog_param.device_id,
                bytes[0], bytes[1], bytes[2], bytes[3], ntohs(query_catalog_param.rmt_port));
    snprintf(temp_buf, sizeof(temp_buf), "<sip:%s@%s>", server_id, domain_id);
    ret = generate_request(&dest, GBSIP_METHOD_MESSAGE, to, temp_buf, NULL);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "response catalog msg to str failed:%d\n", ret);
        return -1;
    }

    snprintf(temp_buf, sizeof(temp_buf), "%s/%s", CONTENT_TYPE_APPLICATION, CONTENT_SUBTYPE_MANSCDP_XML);
    osip_message_set_content_type(dest, temp_buf);

    snprintf(temp_buf, sizeof(temp_buf), "<?xml version=\"1.0\" encoding=\"gb2312\"?>\r\n<Response>\n<CmdType>Catalog</CmdType>\n");
    temp_len = strlen(temp_buf);
       
    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<SN>%u</SN>\n<DeviceID>%s</DeviceID>\n<SumNum>%u</SumNum>\n",
                query_catalog_param.sn, query_catalog_param.device_id, query_catalog_param.num);
    temp_len = strlen(temp_buf);
           
    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<DeviceList Num=\"%u\">\n",
                catalog_num);
    temp_len = strlen(temp_buf);
    if (catalog_num > 0)
    {
        for(int i = 0; i < catalog_num; i++)
        {
            MN::CCatalogItem catalog_item;
            memcpy(&catalog_item, (uint8_t *)catalog_buf + i * sizeof(catalog_item), sizeof(catalog_item));

            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<Item>\n<DeviceID>%s</DeviceID>\n<Name>%s</Name>\n",
                        catalog_item.device_id, catalog_item.name);
            temp_len = strlen(temp_buf);
        
            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<Manufacturer>%s</Manufacturer>\n<Model>%s</Model>\n<Owner>%s</Owner>\n",
                        catalog_item.manufacturer, catalog_item.model, catalog_item.owner);
            temp_len = strlen(temp_buf);

            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "<CivilCode>%s</CivilCode>\n<Address>%s</Address>\n<Parental>%u</Parental>\n",
                        catalog_item.civil_code, catalog_item.address, catalog_item.parental);
            temp_len = strlen(temp_buf);

            snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len,
                        "<SafetyWay>%u</SafetyWay>\n<RegisterWay>%u</RegisterWay>\n<Secrecy>%u</Secrecy>\n<Status>%s</Status>\n</Item>\n",
                        catalog_item.safe_way, catalog_item.register_way, catalog_item.secrecy, catalog_item.on ? "ON" : "OFF");
            temp_len = strlen(temp_buf);
        }    
    }

    snprintf(temp_buf + temp_len, sizeof(temp_buf) - temp_len, "</DeviceList>\n</Response>\n");
    temp_len = strlen(temp_buf);
           
    osip_message_set_body(dest, temp_buf, temp_len);

    // query device catalog
    ret = send_data(dest, query_catalog_param.rmt_ip, query_catalog_param.rmt_port);
    osip_message_free(dest);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send catalog response failed:%d\n", ret);
    }

    return ret;
}

/// response to remote INVITE Cmd
int CGbExosip::rmt_invite(MN::CMediaInvite &invite_param, osip_message_t **res)
{
    int ret;
    osip_message_t *answer;
    osip_message_t *msg     = (osip_message_t *)invite_param.ext_data;
    ESipResCode code        = SIP_RESPONSE_200_OK;

    if ( (0 == invite_param.local_net_info.rtp_port) || (0 == invite_param.local_net_info.rtp_ip) )
    {
        code = SIP_RESPONSE_503_SERVEICE_UNAVAILABEL;
    }

    // build response
    ret = build_response(&answer, code, msg);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "build answer failed:%d\n", ret);
    }
    
    /// add msg body
    if (SIP_RESPONSE_200_OK == code)
    {
        snprintf(temp_buf, sizeof(temp_buf), "%s/%s", CONTENT_TYPE_APPLICATION, CONTENT_SUBTYPE_SDP);
        osip_message_set_content_type(answer, temp_buf);

        snprintf(temp_buf, sizeof(temp_buf), "v=0\no=%s 0 0 IN IP4 %s\n%s\n%s\nm=video %u RTP/AVP %u\n",
                    server_id, server_ip, "s=Network Video Server", "t=0 0", invite_param.local_net_info.rtp_port, invite_param.payload_type);
        temp_len = strlen(temp_buf);

        snprintf(temp_buf + temp_len , sizeof(temp_buf) - temp_len , "%s\na=rtpmap:%u PS/90000\na=username:%s\n%s\n%s\ny=0%u\n",
                    "a=sendonly", invite_param.payload_type, server_id, "a=password:123456", "a=filesize:0", invite_param.ssrc);
        temp_len = strlen(temp_buf);

        osip_message_set_body(answer, temp_buf, temp_len);
    }

    // query device catalog
    ret = send_data(answer, invite_param.dev_ip, invite_param.dev_port);
    osip_message_free(msg);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "send catalog response failed:%d\n", ret);
        osip_message_free(answer);
        return -1;
    }

    if (NULL != res)
    {
        *res    = answer;
    }
    return 0;
}

/// 设置发送回调
void CGbExosip::set_send_data_cb(FSendBufCb buf_cb, void *buf_ctx, FSendMsgCb msg_cb, void *msg_ctx)
{
    snd_buf_cb      = buf_cb;
    snd_buf_cb_ctx  = buf_ctx;
    snd_msg_cb      = msg_cb;
    snd_msg_cb_ctx  = msg_ctx;
}
