#ifndef __GB_SERVER_H__
#define __GB_SERVER_H__

#include "gbexosip.h"
#include "xmlparser.h"
#include "media_cbb.h"
#include "sdp.h"
#include "session.h"
#include <map>

class CGbExosip;
class CGbMsgQueue;
class GbRegSession;     /// 
class CGbLink;

class CGbCatalog;


class CGbServer
{
    friend class CGbRegSession;
public:
    CGbServer();    /// 构造函数

    ~CGbServer();   /// 析构函数


public:
    static int destroy(gb_handle_t handle);

    static int start(gb_handle_t handle);

    static int set_ev_cb(gb_handle_t handle, void *context, int (*cb)(void *ctx, int type, void *var, int var_len));

    static int invite(gb_handle_t handle, MN::CMediaInvite &invite_param);

    static int reponse_catalog(gb_handle_t handle, MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len);

    static int rmt_invite(gb_handle_t handle, MN::CMediaInvite &invite_param);

    static int init();

    int create(GB::CGbSvrParam &param);

    int rmt_invite(MN::CMediaInvite &invite_param);

    int invite(MN::CMediaInvite &invite_param);

    int reponse_catalog(MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len);

    int start();

    int set_ev_cb(void *context, int (*cb)(void *ctx, int type, void *var, int var_len));

    static void show_server();

    static void show_dlg();
    
private:

    /// cata log res callback
    ESipResCode add_catalog_query_result(TManscdpQueryCatalogRes *catalog_res, uint32_t ip = 0, uint16_t port = 0, uint64_t time = 0);

    /// record info callback
    ESipResCode add_recordinfo_query_result(CManscdpResRecordInfo *rec_info, uint32_t ip, uint16_t port, uint64_t time);

    /*
     * nc query record info, callback to msgcenter
     */
    ESipResCode rmt_query_record_info(TManscdpXmlResult *result, uint32_t ip, uint16_t port, uint64_t time);

    // add server to list
    void    add_to_list();

    // delete server from list
    void del_from_list();

    // set sip event callback
    void set_sip_event_cb();

    // deal xml data
    ESipResCode deal_xml(void *buf, int len);

    // deal sdp data
    ESipResCode deal_sdp(void *buf, int len);

    /*
     * update xml result
     */
    ESipResCode update_xml_result(TManscdpXmlResult *result, uint32_t ip = 0, uint16_t port = 0, uint64_t time = 0);

    /*
     * server/nc query catalog
     */
    ESipResCode rmt_query_catalog(TManscdpXmlResult *result);

    // typedef ESipResCode (*FEvCb)(void *context, int type, void *buf, int len);
    static ESipResCode sip_event_deal(void *context, int type, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);

    /// callback event
    int call_back(int type, void *var, int var_len);

    /// app callback event,发送数据到消息中心
    ESipResCode app_cb(int type, void *buf, int len, bool add_queue = false);

    /// 处理带有manscdp+xml消息体的MESSAGE请求
    ESipResCode deal_manscdp_msg(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// 处理manscdp信息
    ESipResCode manscdp_parse(char *buf, uint32_t ip, uint16_t port, uint64_t time);

    /// 处理消息中心发来的消息
    void deal_inter_msg(void *buf, int buf_len, char *rmt_addr, uint64_t cur_time);

    /// link sip server data callback
    static void link_svr_data_cb(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t cur_time);

    /// 发送函数
    static int link_svr_buf_send(void *context, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time);

    /// 发送函数回调
    static int link_svr_msg_send(void *context, void *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// link inter msg data callback
    static void link_inter_msg_cb(void *context, void *buf, int buf_len, char *rmt_addr, uint64_t cur_time);

    /// link external msg data callback
    static void link_exteranl_data_cb(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t time);

    /// link ncproxy msg data callback
    static void link_ncproxy_data_cb(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t time);

    /// link trigger
    static void link_trigger_cb(void *context, uint64_t cur_time);

    /// 查询目录结构
    int query_catalog(osip_message_t *msg, uint32_t ip, uint16_t port);

    /// 处理媒体请求信令
    ESipResCode deal_rmt_invite(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// 处理对端媒体请求确认信息
    void deal_rmt_ack(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// deal remote BYE request 
    ESipResCode deal_rmt_bye(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time);

    /// deal remote REGISTER request
    ESipResCode deal_rmt_register(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time, bool authed = false);

private:
    GB::CGbSvrParam     cfg_param;          /// cfg data for server
    CGbExosip           *exosip;            /// exosip obj
    CGbLink             *linker;            /// 链路层对象
    HXmlParser          xml_parser;         /// xml parser obj

    CGbRegSession       *reg_session;           /// 注册会话
    CGbCatalog          *catalog_session;       /// 条目资源会话


    

    static CGbServer                *server_list;                  // list for all server
    static gb_handle_t              handle_list[1];
    static pthread_mutex_t          server_list_lock;  // list lock 

    // to callback data
    uint32_t                        msg_sn;
    char                            buf[8192];
    int                             buf_len;

    char                            app_buf[8192];
    int                             app_buf_len;

    // callback & ctx
    int                             (*ev_cb)(void *ctx, int type, void *var, int var_len);
    void                            *ev_cb_context ;
    uint64_t                        cur_wall_clk_time;

    uint32_t                        osip_rmt_ip;
    uint16_t                        osip_rmt_port;

    SDP::TSdpInfo                   sdp_info;
    SDP::TSdpMediaDesc              media_desc;


    char                            server_id[21];

    char                            msg_center_addr[64];

    CGbMsgQueue                     *msg_queue;
    class CGbServer     *next;          // next server

    uint64_t                        last_trigger_time;

    /// osip related
    osip_message_t                 *cur_msg;
    static osip_t                  *osip_handle;
};


#endif  // __GB_SERVER_H__
