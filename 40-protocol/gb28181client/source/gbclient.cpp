
#ifdef _MSC_VER

#else

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif

#include <mutex>

#include "oscbb.h"
#include "common.h"
#include "gb.h"
#include "link.h"
#include "gbexosip.h"
#include "xmlparser.h"
#include "catalog.h"
#include "recinfo.h"

class CGbClient
{
public:
    CGbClient();

    /// 初始化
    static int init();

    /// 创建国标客户端
    int create(GB::CGbClientParam &param);

    /// 开启国标客户端
    int start();

    /// 停止国标客户端
    int stop();

    /// 设置参数
    int setparam(GB::CGbClientParam &param);

    /// 获取参数
    int getparam(GB::CGbClientParam &param);

    /// 注册
    int reg_to_server(const GB::CGbRegParam &reg_param);

    /// 设置回调函数
    int set_cb(void *context, int (*cb)(void *ctx, int type, void *var, int var_len));

    /// 查询目录结构
    int query_catalog();

    /// 媒体请求
    int invite_media(char *media_sender_id, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype);

    /// 历史媒体请求
    int invite_history_media(char *media_sender_id, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype);

    /// media BYE op
    int bye_media(char *media_sender_id, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype);

    /* query record info
    * @secrecy:0 or 1
    */
    int query_record_info(GB::CGbRecordQueryParam &query_param);

private:
    CLink       *link;
    CGbExosip   *exosip;
    HXmlParser  xml_parser;
    CGbCatalog  *catalog_queue;
    CRecordInfoList     *record_info_list;

    /// sip信令处理函数
    static void sip_msg_cb(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t cur_time);

    /// 释放资源
    void free_resource();

    /// SIP 事件回调
    static ESipResCode exosip_ev_cb(void *context, int type, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);
    int reg_to_server_with_auth(osip_message_t *msg);
    static int link_client_buf_send(void *context, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);
    static int link_client_msg_send(void *context, void *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// 解析manscdp内容
    ESipResCode manscdp_parse(osip_message_t *manscdp, uint32_t ip, uint16_t port, uint64_t time);

    /*
     * deal xml result
     */
    ESipResCode deal_manscdp_result(osip_message_t *msg, TManscdpXmlResult *result, uint32_t ip, uint16_t port, uint64_t time);

    /// cata log res callback
    ESipResCode add_catalog_query_result(TManscdpQueryCatalogRes *catalog_res, uint32_t ip, uint16_t port, uint64_t time);

    ///video info 
    ESipResCode add_record_info_query_result(CManscdpResRecordInfo *vedio_info_res, uint32_t ip, uint16_t port, uint64_t time);

    /// 处理媒体请求响应
    int deal_invite_response(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time);

    uint32_t rmt_ip;
    uint16_t rmt_port;
    uint32_t local_ip;
    uint32_t local_port;

    uint64_t cur_time;

    uint32_t c_seq;
    uint32_t manscdp_seq;

    uint8_t tmp_buf[2048];


    GB::CGbClientParam local_cfg_param;
    GB::CGbRegParam reg_param;

    int     (*usr_cb)(void *ctx, int type, void *var, int var_len);
    void    *usr_cb_ctx;

    enum client_status_t
    {
        ECLIENT_STATUS_UNKNOW = 0,
        ECLIENT_STATUS_INIT,
        ECLIENT_STATUS_REGISTING,

        ECLIENT_STATUS_REGISTERED,
        ECLIENT_STATUS_DEINIT,
    }fsm_status;

    static std::once_flag   osip_init_flag;
    static osip_t           *osip_handle;
};

osip_t *CGbClient::osip_handle = NULL;
std::once_flag CGbClient::osip_init_flag;

CGbClient::CGbClient()
{
    rmt_ip      = 0;
    rmt_port    = 0;
    local_ip    = 0;
    local_port  = 0;

    usr_cb      = NULL;
    usr_cb_ctx  = NULL;

    cur_time    = 1;
    c_seq       = 1;
    manscdp_seq = 1;
    xml_parser  = XMLPARSER_INVALID_HANDLE;

    catalog_queue   = NULL;
    fsm_status      = ECLIENT_STATUS_UNKNOW;
}


/// GB鏈嶅姟鍣ㄧ粍浠跺垵濮嬪寲
int CGbClient::init()
{
    int ret = osip_init(&osip_handle);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip init failed\n");
        return -1;
    }

    CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip init successfully\n");

    return 0;
}


/// 创建国标客户端
int CGbClient::create(GB::CGbClientParam &param)
{
    std::call_once(osip_init_flag, [](){
        if (CGbClient::init() < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip init failed\n");
        }

        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip init successfully\n");
        });

    try
    {
        link    = new CLink();
        exosip  = new CGbExosip(param.port, param.sip_id, param.domain_id, param.user_agent, param.ip);
        catalog_queue       = new CGbCatalog;
        record_info_list    = new CRecordInfoList;
    }
    catch (...)
    {
        free_resource();
        return GB_ERR_NOMEM;
    }

    xml_parser = xml_create();
    if (XMLPARSER_INVALID_HANDLE == xml_parser)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "xml parser create failed\n");
        free_resource();
        return GB_ERR_NOMEM;
    }

    exosip->set_ev_cb(exosip_ev_cb, this);
    exosip->set_send_data_cb(link_client_buf_send, this, link_client_msg_send, this);

    int ret = catalog_queue->create();
    if (0 != ret)
    {
        free_resource();
        return GB_ERR_NOMEM;
    }

    local_ip    = inet_addr(param.ip);
    cur_time    = 1;
    c_seq       = 1;

    CLink::CCfgParam client_cfg_param;
    client_cfg_param.client_addr = inet_addr(param.ip);
    client_cfg_param.client_port    = htons(param.port);
    client_cfg_param.client_data_cb = CGbClient::sip_msg_cb;
    client_cfg_param.client_data_cb_ctx = this;
    ret = link->create(client_cfg_param);
    if (0 != ret)
    {
        free_resource();
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "gbclient link create failed\n");
        return ret;
    }
    local_cfg_param = param;
    fsm_status = ECLIENT_STATUS_INIT;

    return 0;
}

int CGbClient::start()
{
    return 0;
}



int CGbClient::stop()
{
    return 0;
}

int CGbClient::setparam(GB::CGbClientParam & param)
{
    return 0;
}

int CGbClient::getparam(GB::CGbClientParam & param)
{
    return 0;
}

void CGbClient::sip_msg_cb(void *context, void *data, int data_len, uint32_t ip, uint16_t port, uint64_t time)
{
    CGbClient *client   = static_cast<CGbClient *>(context);

    // sip msg must bigger than 32
    if (data_len < 33)
    {
        return;
    }

    client->rmt_ip      = ip;
    client->rmt_port    = port;
    client->cur_time    = time;

    client->exosip->data_in(data, data_len, ip, port, time);
}

namespace
{
#include <osipparser2/osip_md5.h>
#define HASHLEN 16
typedef char HASH[HASHLEN];

#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];

void CvtHex(HASH Bin, HASHHEX Hex)
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
          Hex[i * 2] = (j + '0');
        else
          Hex[i * 2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
          Hex[i * 2 + 1] = (j + '0');
        else
          Hex[i * 2 + 1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
void digest_calc_ha1(const char *pszAlg,
                    const char *pszUserName, unsigned int username_len,
                    const char *pszRealm, unsigned int realm_len,
                    const char *pszPassword, unsigned int passwd_len,
                    const char *pszNonce, const char *pszCNonce, HASHHEX SessionKey)
{
    osip_MD5_CTX Md5Ctx;
    HASH HA1;
    HASHHEX HA1Hex;

    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszUserName, username_len);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszRealm, realm_len);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszPassword, passwd_len);
    osip_MD5Final ((unsigned char *) HA1, &Md5Ctx);
    if ((pszAlg != NULL) && osip_strcasecmp (pszAlg, "md5-sess") == 0) {
        CvtHex(HA1, HA1Hex);
        osip_MD5Init (&Md5Ctx);
        osip_MD5Update(&Md5Ctx, (unsigned char *)HA1Hex, HASHHEXLEN);
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, (unsigned int) strlen (pszNonce));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, (unsigned int) strlen (pszCNonce));
        osip_MD5Final ((unsigned char *) HA1, &Md5Ctx);
    }
    CvtHex (HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void digest_calc_response(HASHHEX HA1,     /* H(A1) */
                    const char *pszNonce, unsigned int nonce_len,    /* nonce from server */
                    const char *pszNonceCount,       /* 8 hex digits */
                    const char *pszCNonce,   /* client nonce */
                    const char *pszQop,      /* qop-value: "", "auth", "auth-int" */
                    int Aka, /* Calculating AKAv1-MD5 response */
                    const char *pszMethod,   /* method from the request */
                    const char *pszDigestUri, unsigned int uri_len,        /* requested URL */
                    HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
                    HASHHEX Response /* request-digest or response-digest */ )
{
    osip_MD5_CTX Md5Ctx;
    HASH HA2;
    HASH RespHash;
    HASHHEX HA2Hex;

    /* calculate H(A2) */
    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszMethod, (unsigned int) strlen (pszMethod));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszDigestUri, uri_len);

    if (pszQop == NULL) {
        goto auth_withoutqop;
    }
    else if (0 == osip_strcasecmp (pszQop, "auth-int")) {
        goto auth_withauth_int;
    }
    else if (0 == osip_strcasecmp (pszQop, "auth")) {
        goto auth_withauth;
    }

auth_withoutqop:
    osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
    CvtHex (HA2, HA2Hex);

    /* calculate response */
    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) HA1, HASHHEXLEN);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, nonce_len);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);

    goto end;

auth_withauth_int:

    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) HEntity, HASHHEXLEN);

auth_withauth:
    osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
    CvtHex (HA2, HA2Hex);

    /* calculate response */
    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) HA1, HASHHEXLEN);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, (unsigned int) strlen (pszNonce));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    if (Aka == 0) {
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonceCount, (unsigned int) strlen (pszNonceCount));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, (unsigned int) strlen (pszCNonce));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
        osip_MD5Update (&Md5Ctx, (unsigned char *) pszQop, (unsigned int) strlen (pszQop));
        osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    }
end:
    osip_MD5Update (&Md5Ctx, (unsigned char *) HA2Hex, HASHHEXLEN);
    osip_MD5Final ((unsigned char *) RespHash, &Md5Ctx);
    CvtHex (RespHash, Response);
}
}


/// 注册,带数字认证注册
int CGbClient::reg_to_server_with_auth(osip_message_t *msg)
{
    osip_www_authenticate_t *www_auth = (osip_www_authenticate_t *)msg->www_authenticates.node->element;

    HASHHEX  ha1;
    HASHHEX  response;

#define key_value_len(key)  \
        char *key = www_auth->key;  \
        if ('"' == key[0])    \
        {                       \
            key++;   \
        }   \
        unsigned int v_len_##key = strlen(key);   \
        if ( ('"' == key[v_len_##key - 1]) && (v_len_##key > 1) )    \
        {                               \
            v_len_##key--;         \
        }

        key_value_len(realm);

    char *username = local_cfg_param.sip_id;
    int v_len_username  = strlen(username);
    digest_calc_ha1("md5", username, v_len_username, realm, v_len_realm, reg_param.passwd, strlen(reg_param.passwd), www_auth->nonce, NULL, ha1);

    key_value_len(nonce);
    char uri[64];
    snprintf(uri, sizeof(uri), "sip:%s@%s", reg_param.server_sip_id, reg_param.server_domain_id);
    digest_calc_response(ha1, nonce, v_len_nonce, NULL, NULL, NULL, 0, GBSIP_METHOD_REGISTER, uri, strlen(uri), NULL, response);

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header),
        "Authorization: Digest username=\"%s\", realm=\"%s, nonce=\"%s, uri=\"%s\", response=\"%s\", algorithm=MD5\r\n",
        username, realm, nonce, uri, response);

    osip_generic_param_t *tag;

    osip_from_get_tag (msg->from, &tag);

    return exosip->reg_to_server(const_cast<char *>(reg_param.server_sip_id), const_cast<char *>(reg_param.server_domain_id),
                                    local_cfg_param.sip_id, local_cfg_param.domain_id,
                                    local_ip, local_cfg_param.port, reg_param.server_ip, reg_param.server_port,
                                    c_seq++, auth_header, tag->gvalue, msg->call_id->number);
}


/// 注册
int CGbClient::reg_to_server(const GB::CGbRegParam &param)
{
    fsm_status = ECLIENT_STATUS_REGISTING;
    reg_param   = param;

    return exosip->reg_to_server(const_cast<char *>(param.server_sip_id), const_cast<char *>(param.server_domain_id),
                                local_cfg_param.sip_id, local_cfg_param.domain_id, local_ip, local_cfg_param.port,
                                param.server_ip, param.server_port, c_seq++);
}

/* query record info
* @secrecy:0 or 1
*/
int CGbClient::query_record_info(GB::CGbRecordQueryParam &query_param)
{
    int ret = record_info_list->insert_ctx_sn(query_param.user_ctx, manscdp_seq);
    if (GB_SUCCESS != ret)
    {
        return ret;
    }

    /// construct and send sip msg
    ret = exosip->query_record_info(const_cast<char *>(reg_param.server_sip_id), const_cast<char *>(reg_param.server_domain_id),
        local_cfg_param.sip_id, local_cfg_param.domain_id, local_ip, local_cfg_param.port,
        reg_param.server_ip, reg_param.server_port, c_seq++, manscdp_seq++, query_param);

    /// if failed
    if (GB_SUCCESS != ret)
    {
        /// delete current query from record info list
        record_info_list->delete_ctx(manscdp_seq - 1);
    }

    return ret;
}

/// 查询目录结构
int CGbClient::query_catalog()
{
    if ( (ECLIENT_STATUS_UNKNOW == fsm_status) || (ECLIENT_STATUS_INIT == fsm_status) ||
            (ECLIENT_STATUS_REGISTING == fsm_status) || (ECLIENT_STATUS_DEINIT == fsm_status) )
    {
        return GB_ERR_ACCESS;
    }

    return exosip->query_catalog(const_cast<char *>(reg_param.server_sip_id), const_cast<char *>(reg_param.server_domain_id),
                                    local_cfg_param.sip_id, local_cfg_param.domain_id, local_ip, local_cfg_param.port,
                                    reg_param.server_ip, reg_param.server_port, c_seq++, manscdp_seq++);
}

/// media BYE op
int CGbClient::bye_media(char *media_sender_id, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype)
{
    int ret;
    char *call_id = NULL, *local_tag = NULL, *rmt_tag = NULL;

    /// delete local invite dialog
    ret = catalog_queue->del_local_invite_dialog(media_sender_id, &call_id, &local_tag, &rmt_tag, ssrc, media_recv_port, ptype);
    if (0 != ret)
    {
        return GB_ERR_ARG;
    }

    if ( (NULL == call_id) || (NULL == local_tag) || (NULL == rmt_tag) )
    {
        osip_free(call_id);
        osip_free(local_tag);
        osip_free(rmt_tag);
        return GB_ERR_ARG;
    }
    
    char domain_id[8];
    snprintf(domain_id, sizeof(domain_id), "%s", media_sender_id);

    /// BYE media op
    ret = exosip->bye_media(media_sender_id, domain_id, local_cfg_param.sip_id, local_cfg_param.domain_id,
                reg_param.server_ip, reg_param.server_port, c_seq++,
                call_id, local_tag, rmt_tag);

    /// free allocated memory
    osip_free(call_id);
    osip_free(local_tag);
    osip_free(rmt_tag);

    return 0;
}


/// 媒体请求
int CGbClient::invite_media(char *media_sender_id, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype)
{
    uint32_t item_id, dev_id, dlg_id;

    /// local invite media
    int ret = catalog_queue->add_local_invite_dialog(media_sender_id, item_id, dev_id, dlg_id, ssrc, media_recv_port, ptype);
    if (-EEXIST == ret)
    {
        return GB_SUCCESS;
    }
    else if (0 != ret)
    {
        return GB_ERR_BUSY;
    }

    char domain_id[8];
    snprintf(domain_id, sizeof(domain_id), "%s", media_sender_id);

    /// invite media,here need to get local-tag and call-id
    char *local_tag = NULL, *call_id = NULL;
    ret = exosip->invite_media(media_sender_id, domain_id, local_cfg_param.sip_id, local_cfg_param.domain_id,
                                local_ip, local_cfg_param.port, reg_param.server_ip, reg_param.server_port,
                                c_seq++, ssrc, media_recv_port, ptype, &local_tag, &call_id);
    if (0 != ret)
    {
        catalog_queue->del_local_invite_dialog(item_id, dlg_id);
        return ret;
    }

	/// set local invite media param 
    ret = catalog_queue->set_dialog_param(dlg_id, NULL, local_tag, call_id);
    osip_free(local_tag);
    osip_free(call_id);

    return ret;
}

int CGbClient::invite_history_media(char *media_sender_id, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype)
{
    uint32_t item_id, dev_id, dlg_id;

    /// local invite history media
    int ret = catalog_queue->add_local_invite_dialog(media_sender_id, item_id, dev_id, dlg_id, ssrc, media_recv_port, ptype);
    if (-EEXIST == ret)
    {
        return GB_SUCCESS;
    }
    else if (0 != ret)
    {
        return GB_ERR_BUSY;
    }

    char domain_id[8];
    snprintf(domain_id, sizeof(domain_id), "%s", media_sender_id);

    /// invite media,here need to get local-tag and call-id
    char *local_tag = NULL, *call_id = NULL;
    ret = exosip->invite_media(media_sender_id, domain_id, local_cfg_param.sip_id, local_cfg_param.domain_id,
        local_ip, local_cfg_param.port, reg_param.server_ip, reg_param.server_port,
        c_seq++, ssrc, media_recv_port, ptype, &local_tag, &call_id);
    if (0 != ret)
    {
        catalog_queue->del_local_invite_dialog(item_id, dlg_id);
        return ret;
    }

    /// set local invite media param 
    ret = catalog_queue->set_dialog_param(dlg_id, NULL, local_tag, call_id);
    osip_free(local_tag);
    osip_free(call_id);

    return ret;
}



void CGbClient::free_resource()
{
    CBB_SAFE_DELETE(link)
    CBB_SAFE_DELETE(exosip)
    CBB_SAFE_DELETE(catalog_queue)
    CBB_SAFE_DELETE(record_info_list)

    fsm_status = ECLIENT_STATUS_DEINIT;
    usr_cb      = NULL;
    usr_cb_ctx  = NULL;
}

int CGbClient::link_client_buf_send(void *context, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time)
{
    CGbClient   *client         = static_cast<CGbClient *>(context);
    return client->link->sip_send_data(buf, len, htonl(ip), htons(port));
}

/// 鍙戦?佸嚱鏁板洖璋?
int CGbClient::link_client_msg_send(void *context, void *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    return 0;
}


/// SIP 事件回调
ESipResCode CGbClient::exosip_ev_cb(void *context, int type, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time)
{
    CGbClient   *client         = static_cast<CGbClient *>(context);
    ESipResCode code = SIP_RESPONSE_200_OK;
    int ret = 0;

    switch(type)
    {
        case CGbExosip::EV_TYPE_RES_UNAUTH_REG:
            ret = client->reg_to_server_with_auth((osip_message_t *)buf);
            break;

        case CGbExosip::EV_TYPE_MSG_MANSCDPXML:
            code = client->manscdp_parse((osip_message_t *)buf, ip, port, time);

            break;

        case CGbExosip::EV_TYPE_RES_AUTH_REG:
            client->fsm_status  =   ECLIENT_STATUS_REGISTERED;
            if (client->usr_cb)
            {
                client->usr_cb(client->usr_cb_ctx, GB::ECLIENT_EVT_REGISTERED, NULL, 0);
            }
            break;

        case CGbExosip::EV_TYPE_RES_INVITE:
            client->deal_invite_response((osip_message_t *)buf, ip, port, time);
            break;

        case CGbExosip::EV_TYPE_MSG_QUERY_RECORD_INFO:
            code = client->manscdp_parse((osip_message_t *)buf, ip, port, time);
            break;

        default:
            break;
    }

    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "gbclient exosip_ev_cb failed:%s\n", ret);
    }

    return code;
}

/// 瑙ｆ瀽娑堟伅浣撲腑鐨凪ANSCDP淇℃伅
ESipResCode CGbClient::manscdp_parse(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    TManscdpXmlResult       *xml_result;

    osip_body_t *body_tmp = NULL;
    char        *body = NULL;
    size_t       body_len;
    if ( (OSIP_SUCCESS != osip_message_get_body(msg, 0, &body_tmp)) || (NULL == body_tmp) )
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip get body from msg failed\n");
        return SIP_RESPONSE_485_AMBIGUOUS;
    }

    if ( (0 != osip_body_to_str(body_tmp, &body, &body_len)) || (NULL == body) )
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip body to str failed\n");
        return SIP_RESPONSE_485_AMBIGUOUS;
    }

    if (strlen(body) >= sizeof(tmp_buf))
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip body too big\n");

        /// free body resource
        osip_free(body);
        return SIP_RESPONSE_485_AMBIGUOUS;
    }

    memcpy(tmp_buf, body, body_len);
    tmp_buf[body_len]   = 0;

    /// free body resource
    osip_free(body);

    ::xml_parse_text(xml_parser, (char *)tmp_buf, &xml_result);
    if (NULL != xml_result)
    {
        ESipResCode code = deal_manscdp_result(msg, xml_result, ip, port, time);
        ::xml_free_manscdp_result(xml_result);

        return code;
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "xml parser failed,len:%d,data:%s\n", strlen((char *)tmp_buf), (char *)tmp_buf);
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "*************************************\n");
    }

    return SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
}

ESipResCode CGbClient::add_record_info_query_result(CManscdpResRecordInfo *record_info_res, uint32_t ip, uint16_t port, uint64_t time)
{
    ESipResCode code = SIP_RESPONSE_200_OK;
    CRecordInfoItem *item = record_info_res->item;

    int sn = record_info_res->sn;

    /// if add failed, TODO:callback specified node 
    CRecordInfoList::CNode *node = NULL;
    int ret = record_info_list->add_record_info(sn, item, &node);
    if ( (GB_SUCCESS != ret) && (NULL == node) )
    {
        return code;
    }

    /// number of item statisfied || force callback
    if ( (node->current_sum_num >= record_info_res->sum_num) || ((GB_SUCCESS != ret) && (NULL != node)) )
    {
        node->sort_by_start_time();

        GB::CGbRecordItem *record_info_item;
        record_info_item            = node->item;
        record_info_item->sn        = sn;
        record_info_item->sum_num   = record_info_res->sum_num;

        if (usr_cb)
        {
            usr_cb(usr_cb_ctx, GB::ECLIENT_EVT_RES_QUERY_RECORD_INFO, record_info_item, sizeof(GB::CGbRecordItem));
        }

        //删除指定sn所对应的目录信息条目
        record_info_list->delete_ctx(node->sn);
    }

    return code;
}

/// cata log res callback
ESipResCode CGbClient::add_catalog_query_result(TManscdpQueryCatalogRes *catalog_res, uint32_t ip, uint16_t port, uint64_t time)
{
    ESipResCode code = SIP_RESPONSE_200_OK;
    TQueryCatalogItem *item = catalog_res->item;
    GB::CGbCatalogItem  cata_log_item;
    int ret;

    while(item)
    {
        strncpy(cata_log_item.device_id, item->device_id, sizeof(cata_log_item.device_id));
        strncpy(cata_log_item.parent_dev_id, catalog_res->device_id, sizeof(cata_log_item.parent_dev_id));
        strncpy(cata_log_item.name, item->name, sizeof(cata_log_item.name));
        strncpy(cata_log_item.manufacturer, item->manufacturer, sizeof(cata_log_item.manufacturer));
        strncpy(cata_log_item.civil_code, item->civil_code, sizeof(cata_log_item.civil_code));
        strncpy(cata_log_item.owner, item->owner, sizeof(cata_log_item.owner));
        strncpy(cata_log_item.address, item->address, sizeof(cata_log_item.address));
        cata_log_item.parental  = item->parental;
        cata_log_item.safe_way   = item->safe_way;
        cata_log_item.register_way  = item->register_way;
        cata_log_item.secrecy       = item->secrecy;
        cata_log_item.status        = item->status;

        ret = catalog_queue->add_res_item(*item, 1, ip, port, time);
        if (0 != ret)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "catalog queue add res item failed:%d\n", ret);
            code = SIP_RESPONSE_486_BUSY_HERE;
            break;
        }

        if (usr_cb)
        {
            usr_cb(usr_cb_ctx, GB::ECLIENT_EVT_RES_CATALOG, &cata_log_item, sizeof(cata_log_item));
        }

        item    = item->next;
    }

    return code;
}

/// 处理媒体请求响应
int CGbClient::deal_invite_response(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    int             ret;
    osip_via_t      *via_tmp;
    char *from, *to, *call_id, *cseq, *via;

    osip_generic_param_t *local_tag, *rmt_tag;
    if ( (0 != osip_from_get_tag(msg->from, &local_tag)) || (0 != osip_from_get_tag(msg->to, &rmt_tag)) )
    {
        return GB_ERR_ARG;
    }
    if ( (NULL == local_tag) || (NULL == rmt_tag) || (NULL == local_tag->gvalue) || (NULL == rmt_tag->gvalue) )
    {
        return GB_ERR_ARG;
    }

    // to response
    ret = osip_message_get_via (msg, 0, &via_tmp);
    if (0 != ret)
    {
        return GB_ERR_ARG;
    }

    if (0 != osip_call_id_to_str(msg->call_id, &call_id))
    {
        return GB_ERR_NOMEM;
    }

    if ( 0 != osip_to_to_str(msg->to, &to))
    {
        goto free_call_id;
    }
    if (0 != osip_from_to_str(msg->from, &from))
    {
        goto free_to;
    }
    if (0 != osip_cseq_to_str(msg->cseq, &cseq))
    {
        goto free_from;
    }
    if (0 != osip_via_to_str(via_tmp, &via))
    {
        goto free_cseq;
    }

    /// check if dialog exist
    if (0 != catalog_queue->add_local_invite_res_dialog(msg->to->url->username, call_id, local_tag->gvalue, rmt_tag->gvalue))
    {
        goto free_via;
    }

    char dst_domain_id[8];
    snprintf(dst_domain_id, sizeof(dst_domain_id), "%s", msg->to->url->username);

    /// 应答媒体请求
    ret = exosip->ack_invite(msg->to->url->username, dst_domain_id, local_cfg_param.sip_id, local_cfg_param.domain_id,
                            local_ip, local_cfg_param.port, reg_param.server_ip, reg_param.server_port,
                            via, call_id, from, to, cseq);

free_via:
    osip_free(via);

free_cseq:
    osip_free(cseq);

free_from:
    osip_free(from);

free_to:
    osip_free(to);

free_call_id:
    /// free resource
    osip_free(call_id);

    return ret;
}


/*
 * deal xml result
 */
ESipResCode CGbClient::deal_manscdp_result(osip_message_t *msg, TManscdpXmlResult *result, uint32_t ip, uint16_t port, uint64_t time)
{
    ESipResCode code = SIP_RESPONSE_420_BAD_EXTENSION;
    int ret;
    char tag[64];
    switch (result->manscdp_type)
    {
        case EMANSCDP_TYPE_CONTROL:
            break;

        case EMANSCDP_TYPE_QUERY:
            switch (result->sub_cmd_type)
            {
                case EMANSCDP_SUB_CMD_CATALOG:
                    // code = rmt_query_catalog(result);
                    break;

                case EMANSCDP_SUB_CMD_DEVICE_INFO:
                    ret = exosip->res_to_message(msg, SIP_RESPONSE_200_OK, tag, sizeof(tag));
                    if (0 == ret)
                    {
                        /// 回调事件
                        /// 应答设备信息
                        snprintf((char *)tmp_buf, sizeof(tmp_buf), "%s%d%s%s%s",
                                "<?xml version=\"1.0\"?>\r\n\r\n"
                                "<Response>\r\n"
                                "<CmdType>DeviceInfo</CmdType>\r\n"
                                "<SN>", result->sn,
                                "</SN>\r\n"
                                "<DeviceID>", result->device_id,
                                "</DeviceID>\r\n"
                                "<Result>OK</Result>\r\n"
                                "<DeviceType>IMOS</DeviceType>\r\n"
                                "<Manufacturer>NETMARCH</Manufacturer>\r\n"
                                "<Model>IMOS beta 1.0</Model>\r\n"
                                "<Firmware>V0.1,build time:</Firmware>\r\n"
                                "</Response>\r\n");
                        ret = exosip->res_to_dev_info(msg, reg_param.server_sip_id, reg_param.server_domain_id,
                                                        htonl(reg_param.server_ip), reg_param.server_port,
                                                        local_cfg_param.sip_id, local_cfg_param.domain_id,
                                                        tag, (char *)tmp_buf);
                    }
                    break;

                default:
                    CLog::log(CLog::CLOG_LEVEL_API, "update xml result,manscdp type:%d,invalid subcmd type:%d\n",
                                result->manscdp_type, result->sub_cmd_type);
                    break;
            }
            break;

        case EMANSCDP_TYPE_NOTIFY:
            code    = SIP_RESPONSE_200_OK;
            break;

        case EMANSCDP_TYPE_RESPONSE:
            switch (result->sub_cmd_type)
            {
                case EMANSCDP_SUB_CMD_CATALOG:
                    code = add_catalog_query_result((TManscdpQueryCatalogRes *)result->value, ip, port, time);

                    ret = exosip->res_to_message(msg, code, ntohl(rmt_ip), ntohs(rmt_port) );
                    break;

                case EMANSCDP_SUB_CMD_RECORD_INFO:
                    code = add_record_info_query_result( (CManscdpResRecordInfo *)result->value, ip, port, time);
                    ret = exosip->res_to_message(msg, code, ntohl(rmt_ip), ntohs(rmt_port));
                    break;

                default:
                    CLog::log(CLog::CLOG_LEVEL_API, "update xml result,manscdp type:%d,invalid subcmd type:%d\n",
                                result->manscdp_type, result->sub_cmd_type);
                    break;
            }
            break;

        default:
            CLog::log(CLog::CLOG_LEVEL_API, "update xml result, invalid manscdp type:%d\n", result->manscdp_type);
            break;
    }

    return code;
}

/// 设置用户回调函数
int CGbClient::set_cb(void *context, int (*cb)(void *ctx, int type, void *var, int var_len))
{
    usr_cb      = cb;
    usr_cb_ctx  = context;
    return 0;
}

























namespace GB
{

    /// 国标客户端接口
    int gb_client_create(gb_handle_t *handle, CGbClientParam &param)
    {
        CGbClient *client;
        try
        {
            client = new CGbClient;
        }
        catch (...)
        {
            return -1;
        }

        int ret = client->create(param);
        if (0 == ret)
        {
            *handle = client;
        }
        else
        {
            delete client;
        }

        return ret;
    }



    int gb_client_start(gb_handle_t handle)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->start();
    }

    int gb_client_stop(gb_handle_t handle)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->stop();
    }


    int gb_client_setparam(gb_handle_t handle, CGbClientParam &param)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->setparam(param);
    }

    int gb_client_getparam(gb_handle_t handle, CGbClientParam &param)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->getparam(param);
    }

    int gb_client_reg_to_server(gb_handle_t handle, const CGbRegParam &reg_param)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->reg_to_server(reg_param);
    }

    /// 国标客户端设置回调函数
    int gb_client_set_cb(gb_handle_t handle, void *context, int (*cb)(void *ctx, int type, void *var, int var_len))
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->set_cb(context, cb);
    }

    /// 请求媒体
    int gb_client_invite_media(gb_handle_t handle, char *media_sender_id,
                                uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->invite_media(media_sender_id, ssrc, media_recv_port, ptype);
    }

    /// close one media
    int gb_client_bye_media(gb_handle_t handle, char *media_sender_id,
                                        uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->bye_media(media_sender_id, ssrc, media_recv_port, ptype);
    }

    /// 查询目录结构
    int gb_client_query_catalog(gb_handle_t handle)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->query_catalog();
    }

    /* query record info
    * @secrecy:0 or 1
    */
    int gb_client_query_record_info(gb_handle_t handle, CGbRecordQueryParam &query_param)
    {
        CGbClient *client = static_cast<CGbClient *>(handle);
        return client->query_record_info(query_param);
    }

}   /// end of namespace GB
