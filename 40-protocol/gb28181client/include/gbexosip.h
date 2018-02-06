
#ifndef __GB_EXOSIP_H__
#define __GB_EXOSIP_H__

#include <stdint.h>

// exosip/osip include
#include <osip2/osip.h>


#define GBSIP_VER    "SIP/2.0"

#define GBSIP_METHOD_REGISTER   "REGISTER"
#define GBSIP_METHOD_MESSAGE    "MESSAGE"
#define GBSIP_METHOD_INVITE     "INVITE"


class CGbExosip
{
    friend class CGbClient;
public:
    static void *routine(void *arg);

public:
    /// 鏋勯?犲嚱鏁?,绔彛涓轰富鏈哄簭
    CGbExosip(uint16_t p, char *id, char *domain, char *agent, char *ip);

    // ev for callback
    enum event_type_t
    {


        EV_TYPE_REQ_REGISTER,   // msgno.h:struct MN_API CDevRegiste
        EV_TYPE_REQ_INVITE,     // msgno.h:struct MN_API CMediaInvite

        EV_TYPE_UNAUTH_REG,     /// 涓嶅甫璁よ瘉鐨勬敞鍐岃姹?
        EV_TYPE_AUTH_REG,       /// 甯﹁璇佺殑娉ㄥ唽璇锋眰

        EV_TYPE_MSG_MANSCDPXML, /// 甯︽湁manscdp+xml鐨勬秷鎭姹?

        EV_TYPE_MSG_SDP,        /// 甯︽湁manscdp+xml鐨勬秷鎭姹?

        EV_TYPE_RES_UNAUTH_REG, /// 非权鉴的注册请求,此处回调sip消息
        EV_TYPE_RES_AUTH_REG,   /// 鉴权通过的注册请求,此处无回调
        EV_TYPE_RES_INVITE,     /// 媒体请求应答

        EV_TYPE_MSG_QUERY_RECORD_INFO,  ///查询录像信息
    };

    /// 浜嬩欢鍥炶皟
    typedef ESipResCode (*FEvCb)(void *context, int type, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);

    /// 璁剧疆浜嬩欢鍥炶皟
    void set_ev_cb(FEvCb cb, void *context);

    /// 鍙戦?佸嚱鏁板洖璋?
    typedef int (*FSendBufCb)(void *context, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);

    /// 鍙戦?佸嚱鏁板洖璋?
    typedef int (*FSendMsgCb)(void *context, void *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// 璁剧疆鍙戦?佸洖璋?
    void set_send_data_cb(FSendBufCb snd_buf_cb, void *snd_buf_ctx, FSendMsgCb snd_msg_cb, void *snd_msg_ctx);

    void get_rmt_deviceid(char *device_id_buf, int device_id_buf_len);

    /// data income from link
    void data_in(void *buf, int buf_len, uint32_t ip, uint16_t port, uint64_t cur_time);

    /// register to server
    int reg_to_server(char *server_id, char *server_domain, char *src_id, char *src_domain,
                                    uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                                    uint32_t seq, char *auth_head = NULL, char *from_tag = NULL, char *call_id = NULL);

    /// response to sip request with method MESSAGE
    int res_to_message(osip_message_t *req, ESipResCode code, char *tag = NULL, uint32_t tag_len = 0);

    int res_to_message(osip_message_t *req, ESipResCode code, uint32_t ip, uint16_t port, char *tag = NULL, uint32_t tag_len = 0);

    /// 应答 设备信息查询 消息
    int res_to_dev_info(osip_message_t *req,
                            char *dst_id, char *dst_domain,
                            uint32_t dst_ip, uint16_t dst_port,
                            char *src_id, char *src_domain,
                            char *tag, char *msg_body);

    /// 查询目录结构
    int query_catalog(char *dst_id, char *dst_domain, char *src_id, char *src_domain, uint32_t src_ip, uint16_t src_port,
                        uint32_t dst_ip, uint16_t dst_port, uint32_t seq, uint32_t manscdp_sn);

    int query_record_info(char *dst_id, char *dst_domain, char *src_id, char *src_domain, uint32_t src_ip, uint16_t src_port,
        uint32_t dst_ip, uint16_t dst_port, uint32_t seq, uint32_t manscdp_sn, GB::CGbRecordQueryParam &query_param);

    /// 媒体请求
    int invite_media(char *media_sender_id, char *dst_domain, char *src_id, char *src_domain,
                            uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                            uint32_t seq, uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype,
                            char **local_tag = NULL, char **call_id = NULL);

    /// 应答媒体请求
    int ack_invite(char *media_sender_id, char *dst_domain, char *src_id, char *src_domain,
                            uint32_t src_ip, uint16_t src_port, uint32_t dst_ip, uint16_t dst_port,
                            char *via, char *call_id, char *from, char *to, char *cseq);

    /// 媒体关闭
    int bye_media(char *media_sender_id, char *dst_domain, char *src_id, char *src_domain,
                            uint32_t dst_ip, uint16_t dst_port, uint32_t seq,
                            char *call_id, char *from, char *to);

private:

    // deal incoming message
    void    deal_request();

    void    deal_invite();
    void    deal_response();
    void    deal_message();

    void    construct_res_register(ESipResCode &code);
    void    deal_register();



    void query_cata_log(char *dst_dev_id);


    /*
     * event callback to server
     */
    ESipResCode call_back(int type, void *buf, int len, uint32_t ip = 0, uint16_t port = 0, uint64_t time = 0);

    /// parse data from link
    void parse_data(void *buf, int len);

    /// get new random
    char *malloc_new_random();

    int build_response(osip_message_t **dest, int status, osip_message_t *request, char *user_agent = NULL);

    /// generate new request to send to rmt
    int generate_request(osip_message_t ** dest, const char *method, const char *to, const char *from, const char *proxy);

    void get_uac_addr();

    int send_data(osip_message_t  *to_send_msg);

    /// send data to peer
    int send_data(osip_message_t *to_send_msg, uint32_t ip, uint16_t port);

    /// send data to peer
    int send_data(char *msg, uint32_t len, uint32_t ip, uint16_t port);

    /// 处理权鉴信息
    void deal_authorization(osip_message_t *msg, char *passwd, char *response);

    /// 鑾峰彇msg娑堟伅绫诲瀷,涓昏鏄В鏋恇ody绫诲瀷
    ESipResCode get_msg_type(int &type);


private:
    osip_message_t  *recv_msg;

    // eXosip_t        *exosip_ctx;
    // eXosip_event_t  *exosip_evt;
    osip_t                  *osip_handle;
    osip_authorization_t *authorizations;

    char        cb_buf[512];

    char        temp_buf[2048];
    int         temp_len;
    char        nonce[32];

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

    char        send_buf[1024];
    uint32_t    send_len = 0;

private:
    static const char *AUTH_TYPE;
    static const char *AUTH_ALGORITHM;
    static const char *CONTENT_TYPE_APPLICATION;
    static const char *CONTENT_SUBTYPE_MANSCDP_XML;
    static const char *CONTENT_SUBTYPE_SDP;
};


#endif  // __GB_EXOSIP_H__
