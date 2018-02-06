
#ifndef __GB_EXOSIP_H__
#define __GB_EXOSIP_H__

// exosip/osip include
#include <osip2/osip.h>

class CGbLink;
class CGbUnauthReg;

class CGbExosip
{
    friend class CGbserver;
public:
    static void *routine(void *arg);

public:
    /// 构造函数,端口为主机序
    CGbExosip(uint16_t p, char *id, char *domain, char *agent, char *ip);

    int start();

    // ev for callback
    enum event_type_t
    {
        EV_TYPE_DATA_MANSCDPXML = 1,

        EV_TYPE_DATA_SDP,



        EV_TYPE_REQ_REGISTER,   // msgno.h:struct MN_API CDevRegiste
        EV_TYPE_REQ_INVITE,     // msgno.h:struct MN_API CMediaInvite
        EV_TYPE_REQ_ACK,        /// 回调ACK请求
        EV_TYPE_REQ_BYE,        /// call back BYE request

        EV_TYPE_UNAUTH_REG,     /// 不带认证的注册请求
        EV_TYPE_AUTH_REG,       /// 带认证的注册请求

        EV_TYPE_MSG_MANSCDPXML, /// 带有manscdp+xml的消息请求
    };

    /// 事件回调
    typedef ESipResCode (*FEvCb)(void *context, int type, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);

    /// 设置事件回调
    void set_ev_cb(FEvCb cb, void *context);

    /// 发送函数回调
    typedef int (*FSendBufCb)(void *context, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);

    /// 发送函数回调
    typedef int (*FSendMsgCb)(void *context, void *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// 设置发送回调
    void set_send_data_cb(FSendBufCb snd_buf_cb, void *snd_buf_ctx, FSendMsgCb snd_msg_cb, void *snd_msg_ctx);

    int rmt_invite(MN::CMediaInvite &invite_param, osip_message_t **res = NULL);

    int invite(MN::CMediaInvite &invite_param);

    int reponse_catalog(MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len);

    void get_rmt_deviceid(char *device_id_buf, int device_id_buf_len);

    /// data income from link
    void data_in(void *buf, int buf_len, uint32_t ip, uint16_t port, uint64_t cur_time);

    /// query rmt cata_log
    int query_catalog(char *dst_dev_id, uint32_t ip, uint16_t port);

    int query_record_info(char *dst_dev_id, char *dst_catalog_id, char *start_time, char *end_time, int sn, uint32_t ip, uint16_t port);

    int reponse_record_info(MN::CMsgGbResRecordInfo &gb_rec_res_info, MN::CMsgGbQueryRecordInfo gb_rec_req_info,
                                    void *item_buf, int item_buf_len);
private:
    // deal cmd and run
    void run();
    
    // deal incoming message 
    void    deal_request();

    void    deal_invite();
    void    deal_ack();

    void    deal_bye();
    void    deal_response();
    void    deal_message();
    
    void    construct_res_register(ESipResCode &code);
    void    deal_register();



    void query_cata_log(char *dst_dev_id);

    /*
     * get body type and content
     */
    ESipResCode parse_body();

    /*
     * event callback to server
     */
    ESipResCode call_back(int type, void *buf, int len, uint32_t ip = 0, uint16_t port = 0, uint64_t time = 0);

    /// parse data from link
    void parse_data(void *buf, int len);

    /// get new random
    char *malloc_new_random();

    int build_response(osip_message_t **dest, int status, osip_message_t *request);

    /// generate new request to send to rmt
    int generate_request(osip_message_t ** dest, const char *method, const char *to, const char *from, const char *proxy);

    void get_uac_addr();

    int send_data(osip_message_t  *to_send_msg);

    /// send data to peer
    int send_data(osip_message_t *to_send_msg, uint32_t ip, uint16_t port);

    /// 获取msg消息类型,主要是解析body类型
    ESipResCode get_msg_type(int &type);
private:
    uint16_t    port;
    pthread_t   pid;

    osip_message_t  *recv_msg;

    // eXosip_t        *exosip_ctx;
    // eXosip_event_t  *exosip_evt;
    osip_t                  *osip_handle;
    osip_authorization_t *authorizations;

    char        cb_buf[512];

    char        temp_buf[8192];
    int         temp_len;
    char        nonce[32];

    int         manscdp_sn;

    char        server_id[21];
    char        domain_id[20];
    char        user_agent[32];

    char        server_ip[32];
    char        server_port[8];

    FEvCb       ev_cb;
    void        *ev_cb_context;

    FSendBufCb  snd_buf_cb;
    void        *snd_buf_cb_ctx;
    FSendMsgCb  snd_msg_cb;
    void        *snd_msg_cb_ctx;

    uint32_t    rmt_ip;
    uint16_t    rmt_port;
    uint64_t    cur_time;
    char        uac_ip[32];
    char        uac_port[8];


private:
    static const char *AUTH_TYPE;
    static const char *AUTH_ALGORITHM;
    static const char *CONTENT_TYPE_APPLICATION;
    static const char *CONTENT_SUBTYPE_MANSCDP_XML;
    static const char *CONTENT_SUBTYPE_SDP;
};


#endif  // __GB_EXOSIP_H__
