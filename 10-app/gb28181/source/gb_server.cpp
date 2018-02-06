#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>


// prj include
#include "debug.h"
#include "common.h"
#include "gb.h"
#include "gblink.h"
#include "reg.h"
#include "gb_server.h"
#include "gbexosip.h"
#include "catalog.h"

using namespace GB;


class CGbMsgQueue
{
public:
    CGbMsgQueue();
    ~CGbMsgQueue();

    /// queue_len must be 2^n
    int create(uint32_t queue_len = 64);
    void destroy();

    bool add(MN::CMsgFormat &msg, void *ctx = NULL);
    bool del(MN::CMsgFormat &msg, void **ctx = NULL);

    bool get_match(int sn, void **ctx);
    bool get_match(MN::CMsgFormat &msg, void **ctx);

private:
    MN::CMsgFormat  *msg_queue;
    bool            *used;
    void            **user_ctx;

    uint32_t        msg_queue_len;
    uint32_t        mask;    
    pthread_mutex_t mutex;

    const static uint32_t MSG_QUEUE_MIN_LEN;
    const static uint32_t MSG_QUEUE_MAX_LEN;

    void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
};
const uint32_t CGbMsgQueue::MSG_QUEUE_MIN_LEN = 16;
const uint32_t CGbMsgQueue::MSG_QUEUE_MAX_LEN = 1024;

bool CGbMsgQueue::del(MN::CMsgFormat &msg, void **ctx)
{
    bool success    = false;

    lock();

    for (uint32_t i = 0; i < msg_queue_len; i++)
    {
        if (true == used[i])
        {
            if (msg.type == msg_queue[i].type)
            {
                used[i] = false;
                *ctx    = user_ctx[i];
                success = true;
            }
        }
    }

    unlock();

    return success;
}

/// add msg
bool CGbMsgQueue::add(MN::CMsgFormat &msg, void *ctx)
{
    bool success    = false;

    lock();

    for (uint32_t i = 0; i < msg_queue_len; i++)
    {
        if (false == used[i])
        {
            used[i]         = true;
            msg_queue[i]    = msg;
            user_ctx[i]     = ctx;
            success         = true;
        }
    }

    unlock();

    return success;
}

/// construction
CGbMsgQueue::CGbMsgQueue()
{
    user_ctx        = NULL;
    msg_queue       = NULL;
    used            = NULL;
    msg_queue_len   = 0;
    mask            = 0;   
}

/// deconstruction
CGbMsgQueue::~CGbMsgQueue()
{
    destroy();
    msg_queue       = NULL;
    msg_queue_len   = 0;
    mask            = 0;   
    used            = NULL;
    user_ctx        = NULL;
}

/// create msgqueue
int CGbMsgQueue::create(uint32_t queue_len)
{
    CBB_SET_LIMIT(queue_len, MSG_QUEUE_MAX_LEN, MSG_QUEUE_MIN_LEN);

    int ret = pthread_mutex_init(&mutex, NULL);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "mutex init failed, ret:%u\n", ret);
        return -ret;
    }

    msg_queue_len   = MSG_QUEUE_MIN_LEN;
    while(true)
    {
        if (queue_len <= msg_queue_len)
        {
            break;
        }

        msg_queue_len <<= 1;
    }
    mask    = msg_queue_len - 1; 

    try
    {
        msg_queue   = new MN::CMsgFormat[msg_queue_len];
        used        = new bool[msg_queue_len]; 
        user_ctx    = new void *[msg_queue_len];
    }
    catch (...)
    {
        delete []msg_queue;
        delete []used;
        CLog::log(CLog::CLOG_LEVEL_API, "new msg queue failed,maybe no mem,queue len:%u\n", msg_queue_len);
        return -ENOMEM;
    }

    for (uint32_t i = 0; i < msg_queue_len; i++)
    {
        used[i]     = false;
        user_ctx[i] = NULL;
    }

    return 0;
}

/// free all resource
void CGbMsgQueue::destroy()
{
    lock();
    delete []msg_queue;
    delete []used;
    delete []user_ctx;
    unlock();
    int ret = pthread_mutex_destroy(&mutex);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "destroy mutex faile,errno:%d\n", ret);
    }
}


// global list to store server
pthread_mutex_t CGbServer::server_list_lock = PTHREAD_MUTEX_INITIALIZER; 
CGbServer *CGbServer::server_list           = NULL;
gb_handle_t  CGbServer::handle_list[1]      = {0};



/// create one server
int CGbServer::create(GB::CGbSvrParam &svr_param)
{
    int ret = 0;
    
    try
    {
        exosip          = new CGbExosip(svr_param.port, svr_param.sip_id, svr_param.domain_id, svr_param.user_agent, svr_param.ip);
        msg_queue       = new CGbMsgQueue;
        linker          = new CGbLink;

        reg_session     = new CGbRegSession;
        catalog_session = new CGbCatalog;
    }
    catch (...)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "maybe no mem\n");
        ret = -ENOMEM; 
        return ret;
    }

    /// 最大资源条目数量
    ret = catalog_session->create(svr_param.max_res_num);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "create catalog queue failed\n");
        return -ENOMEM;
    }

    /// 最大注册设备数量
    ret = reg_session->create(svr_param.max_reg_dev_num);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "create reg queue failed\n");
        return -ENOMEM;
    }

    ret = msg_queue->create(64);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "create msg queue failed\n");
        return -ENOMEM;
    }

    xml_parser = xml_create();
    if (XMLPARSER_INVALID_HANDLE == xml_parser)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "xml parser create failed\n");
        return -ENOMEM;
    }

    /// set sip event deal
    set_sip_event_cb();

    /// create linker
    CGbLink::CCfgParam  param;
    memset(&param, 0, sizeof(param));
    param.svr_addr              = inet_addr(svr_param.ip);                                                   /// sip server param
    param.svr_port              = htons(svr_param.port);
    param.gb_svr_data_cb        = link_svr_data_cb;
    param.gb_svr_data_cb_ctx    = this;

    /// 内部通信地址
    snprintf(param.inter_msg_addr, sizeof(param.inter_msg_addr), "%s", svr_param.inter_addr);    /// unix param
    param.inter_msg_cb          = link_inter_msg_cb;
    param.inter_msg_cb_ctx      = this;

    /// 外部通信地址设定
    param.external_addr         = inet_addr(svr_param.external_addr);       /// 国标服务器配置信令通信地址
    param.external_port         = htons(svr_param.external_port);           /// 国标服务器配置信令通信端口
    param.external_data_cb      = link_exteranl_data_cb;
    param.external_data_cb_ctx  = this;

    /// 客户端代理模块
    param.ncproxy_addr          = inet_addr(svr_param.ncproxy_addr);
    param.ncproxy_port          = htons(svr_param.ncproxy_port);
    param.ncproxy_data_cb       = link_ncproxy_data_cb;
    param.ncproxy_data_cb_ctx   = this;

    /// 触发器
    param.event_trigger_cb      = link_trigger_cb;                                              /// period trigger
    param.ev_trigger_ctx        = this;
    ret = linker->create(param);

    last_trigger_time           = CBB::gettime_ms();
    cfg_param                   = svr_param;

    add_to_list();

    return ret;
}

int CGbServer::destroy(gb_handle_t handle)
{
    CGbServer *server = static_cast<CGbServer *>(handle);
    server->del_from_list();
    delete server->exosip;
    delete server;

    return 0;
}

int CGbServer::start(gb_handle_t handle)
{
    CGbServer *server = static_cast<CGbServer *>(handle);
    return server->start();
}

/// set callback
int CGbServer::set_ev_cb(gb_handle_t handle, void *context, int (*cb)(void *ctx, int type, void *var, int var_len))
{
    CGbServer *server = static_cast<CGbServer *>(handle);
    return server->set_ev_cb(context, cb);
}

/// set callback
int CGbServer::set_ev_cb(void *context, int (*cb)(void *ctx, int type, void *var, int var_len))
{
    ev_cb           = cb;
    ev_cb_context   = context;

    return 0;
}

int CGbServer::invite(gb_handle_t handle, MN::CMediaInvite &invite_param)
{
    CGbServer *server = static_cast<CGbServer *>(handle);
    return server->invite(invite_param);
}

int CGbServer::invite(MN::CMediaInvite &invite_param)
{
    return exosip->invite(invite_param);
}

int CGbServer::start()
{
    return exosip->start();
}

// add to list
void CGbServer::add_to_list()
{
    CGuard::enterCS(CGbServer::server_list_lock);
    this->next  = CGbServer::server_list;
    CGbServer::server_list = this;
    CGuard::leaveCS(CGbServer::server_list_lock);
}

// delete from list
void CGbServer::del_from_list()
{
    CGuard::enterCS(CGbServer::server_list_lock);

    CGbServer *server = CGbServer::server_list;
    if (server == this)
    {
        CGbServer::server_list = this->next;
    }
    else if (NULL != server)
    {
        while (server->next)
        {
            if (server->next == this)
            {
                server->next = this->next;
                break;
            }
            server = server->next;
        }
    }

    CGuard::leaveCS(CGbServer::server_list_lock);
}

// set sip event callback
void CGbServer::set_sip_event_cb()
{
    if (NULL != exosip)
    {
        exosip->set_ev_cb(sip_event_deal, this);
        exosip->set_send_data_cb(link_svr_buf_send, this, link_svr_msg_send, this);
    }
}

ESipResCode CGbServer::app_cb(int type, void *src_buf, int src_buf_len, bool add_queue)
{
    int ret;
    ESipResCode code;
    MN::CMsgFormat      msg;
    msg.type        = type;
    msg.sn          = msg_sn++;
    msg.var_len     = src_buf_len;

    memcpy(buf, &msg, sizeof(msg));
    if (src_buf_len > 0)
    {
        memcpy(buf + sizeof(msg), src_buf, src_buf_len);
    }

    if (true == add_queue)
    {
        if (false == msg_queue->add(msg))
        {
            CLog::log(CLog::CLOG_LEVEL_API, "add msg queue failed\n");
        }
    }

    ret = linker->inter_send_data(buf, sizeof(msg) + src_buf_len, msg_center_addr);

    code = SIP_RESPONSE_200_OK;
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "inter msg callback to msgcenter failed\n");
        code = SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
    }

    return code;
}

/// cata log res callback
ESipResCode CGbServer::add_recordinfo_query_result(CManscdpResRecordInfo *rec_info, uint32_t ip, uint16_t port, uint64_t time)
{
    ESipResCode code = SIP_RESPONSE_200_OK;
    CRecordInfoItem *item = rec_info->item;
    MN::CMsgGbRecordItem gb_rec_item;
    MN::CMsgGbResRecordInfo gb_rec_info;

    strncpy(gb_rec_info.device_id, rec_info->device_id, sizeof(gb_rec_info.device_id));
    strncpy(gb_rec_info.name, rec_info->name, sizeof(gb_rec_info.name));
    gb_rec_info.sum_num = rec_info->sum_num;
    gb_rec_info.ctx     = rec_info->sn;

    CLog::log(CLog::CLOG_LEVEL_API, "[GB]rec info sum num:%d\n", rec_info->sum_num);

    int list_num     = 0;
    app_buf_len     = sizeof(gb_rec_info);
    while(item)
    {
        strncpy(gb_rec_item.device_id, item->device_id, sizeof(gb_rec_item.device_id));
        strncpy(gb_rec_item.name, item->name, sizeof(gb_rec_item.name));
        strncpy(gb_rec_item.file_path, item->file_path, sizeof(gb_rec_item.file_path));
        strncpy(gb_rec_item.address, item->address, sizeof(gb_rec_item.address));
        strncpy(gb_rec_item.recorder_id, item->recorder_id, sizeof(gb_rec_item.recorder_id));
        gb_rec_item.start_time  = item->start_time;
        gb_rec_item.end_time    = item->end_time;
        gb_rec_item.secrecy     = item->secrecy;
        gb_rec_item.type        = item->type;
        list_num++;
        memcpy(app_buf + app_buf_len, &gb_rec_item, sizeof(gb_rec_item));
        app_buf_len             += sizeof(gb_rec_item);

        /// callback
        if (8 == list_num)
        {
            gb_rec_info.list_num = list_num;
            memcpy(app_buf, &gb_rec_info, sizeof(gb_rec_info));

            CLog::log(CLog::CLOG_LEVEL_API, "[GB]MN_GB_RMT_RES_RECORDINFO,list num:%d,app_buf len:%d\n",
                        list_num, app_buf_len);
            code = app_cb(MN_GB_RMT_RES_RECORDINFO, app_buf, app_buf_len);

            list_num            = 0;
            app_buf_len         = sizeof(gb_rec_info);

            if (SIP_RESPONSE_200_OK != code)
            {
                continue;
            }
        }

        item    = item->next;
    }

    /// callback
    if ( (0 != list_num) || (0 == gb_rec_info.sum_num) )
    {
        gb_rec_info.list_num = list_num;
        memcpy(app_buf, &gb_rec_info, sizeof(gb_rec_info));

        CLog::log(CLog::CLOG_LEVEL_API, "[GB]MN_GB_RMT_RES_RECORDINFO, last num:%d, app_buf len:%d\n",
                    list_num, app_buf_len);
        code = app_cb(MN_GB_RMT_RES_RECORDINFO, app_buf, app_buf_len);
    }

    return code;
}

/// cata log res callback
ESipResCode CGbServer::add_catalog_query_result(TManscdpQueryCatalogRes *catalog_res, uint32_t ip, uint16_t port, uint64_t time)
{
    int ret;
    ESipResCode code = SIP_RESPONSE_200_OK;
    TQueryCatalogItem *item = catalog_res->item;
    MN::CCatalogItem    cata_log_item;

    strncpy(cata_log_item.parent_device_id, catalog_res->device_id, sizeof(cata_log_item.parent_device_id));
    while(item)
    {
        strncpy(cata_log_item.device_id, item->device_id, sizeof(cata_log_item.device_id));
        strncpy(cata_log_item.name, item->name, sizeof(cata_log_item.name));
        strncpy(cata_log_item.manufacturer, item->manufacturer, sizeof(cata_log_item.manufacturer));
        strncpy(cata_log_item.civil_code, item->civil_code, sizeof(cata_log_item.civil_code));
        strncpy(cata_log_item.owner, item->owner, sizeof(cata_log_item.owner));
        strncpy(cata_log_item.address, item->address, sizeof(cata_log_item.address));
        cata_log_item.parental      = item->parental;
        cata_log_item.safe_way      = item->safe_way;
        cata_log_item.register_way  = item->register_way;
        cata_log_item.secrecy       = item->secrecy;
        cata_log_item.on            = item->status;
        cata_log_item.ip            = osip_rmt_ip; 
        cata_log_item.port          = osip_rmt_port; 
        
        /// add to GBstack
        ret = catalog_session->add_res_item(*item, 1, ip, port, time);
        if (0 != ret)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "add catalog item failed, ret:%d\n", ret);
            code = SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
            break;
        }

        code = app_cb(MN_GB_CATALOG_RES, &cata_log_item, sizeof(cata_log_item));
        CLog::log(CLog::CLOG_LEVEL_API, "catalog res:%s, status:%d, ip:%x,port:%u\n",
                    cata_log_item.device_id,cata_log_item.on, cata_log_item.ip, ntohs(cata_log_item.port));

        item    = item->next;
    }

    return code;
}


/*
 * nc query record info, callback to msgcenter
 */
ESipResCode CGbServer::rmt_query_record_info(TManscdpXmlResult *result, uint32_t ip, uint16_t port, uint64_t time)
{
    ESipResCode code = SIP_RESPONSE_420_BAD_EXTENSION;

    GB::CGbRecordItem *query_info = (GB::CGbRecordItem *)result->value;
    if (NULL == query_info)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "may be no memory\n");
        return code;
    }

    if (query_info->device_id[0] == 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "invalid extention\n");
        return code;
    }

    /// sendmsg to msgcenter
    MN::CMsgGbQueryRecordInfo   query_param;

    strncpy(query_param.device_id, cur_msg->from->url->username, sizeof(query_param.device_id));
    strncpy(query_param.catalog_id, query_info->device_id, sizeof(query_param.catalog_id));
    strncpy(query_param.recorder_id, query_info->recorder_id, sizeof(query_param.recorder_id));
    strncpy(query_param.address, query_info->address, sizeof(query_param.address));
    query_param.start_time      = query_info->start_time;
    query_param.end_time        = query_info->end_time;
    query_param.type            = query_info->type;
    query_param.secrecy         = query_info->secrecy;
    query_param.indistinct      = query_info->indistinct;

    query_param.rmt_ip          = ip;
    query_param.rmt_port        = port;
    query_param.ctx             = query_info->sn;

    code = app_cb(MN_GB_RMT_QUERY_RECORDINFO, &query_param, sizeof(query_param));
    CLog::log(CLog::CLOG_LEVEL_API, "remote query record info, %x, device_id:%s\n", query_info->device_id[0], query_info->device_id);

    return code;
}

ESipResCode CGbServer::rmt_query_catalog(TManscdpXmlResult *result)
{
    ESipResCode code = SIP_RESPONSE_200_OK;
    MN::CRmtQueryCatalog query_catalog_param;

    exosip->get_rmt_deviceid(query_catalog_param.device_id, sizeof(query_catalog_param.device_id));
    query_catalog_param.rmt_ip      = osip_rmt_ip;
    query_catalog_param.rmt_port    = osip_rmt_port;
    query_catalog_param.sn          = result->sn;

    code = app_cb(MN_GB_RMT_QUERY_CATALOG, &query_catalog_param, sizeof(query_catalog_param));

    return code;
}

/*
 * update xml result
 */
ESipResCode CGbServer::update_xml_result(TManscdpXmlResult *result, uint32_t ip, uint16_t port, uint64_t time)
{
    ESipResCode code = SIP_RESPONSE_420_BAD_EXTENSION;
    switch (result->manscdp_type)
    {
        case EMANSCDP_TYPE_CONTROL:
            break;

        case EMANSCDP_TYPE_QUERY:
            switch (result->sub_cmd_type)
            {
                case EMANSCDP_SUB_CMD_CATALOG:
                    code = rmt_query_catalog(result);
                    break;

                case EMANSCDP_SUB_CMD_RECORD_INFO:
                    code = rmt_query_record_info(result, ip, port, time);
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
                    break;

                case EMANSCDP_SUB_CMD_RECORD_INFO:
                    code = add_recordinfo_query_result((CManscdpResRecordInfo *)result->value, ip, port, time);
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

/*
 * parse xml data
 */
ESipResCode CGbServer::deal_manscdp_msg(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time)
{
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

    cur_msg = msg;
    memcpy(buf, body, body_len);
    buf[body_len]   = 0;
    ESipResCode code = manscdp_parse(buf, ip, port, time);

    /// free body resource
    osip_free(body);
    
    return code;
}
        
/// 解析消息体中的MANSCDP信息
ESipResCode CGbServer::manscdp_parse(char *manscdp, uint32_t ip, uint16_t port, uint64_t time)
{
    TManscdpXmlResult       *xml_result;
    ::xml_parse_text(xml_parser, manscdp, &xml_result);
    if (NULL != xml_result)
    {
        ESipResCode code = update_xml_result(xml_result, ip, port, time);
        ::xml_free_manscdp_result(xml_result);            
        return code;
    }
    else
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "xml parser failed,len:%d,data:%s\n", strlen(manscdp), manscdp);
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "*************************************\n");
    }

    return SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
}

/*
 * parse xml data
 */
ESipResCode CGbServer::deal_xml(void *buf, int len)
{
    char *txt = static_cast<char *>(buf);
    TManscdpXmlResult       *xml_result;
    txt[len] = 0;
    ::xml_parse_text(xml_parser, txt, &xml_result);
    if (NULL != xml_result)
    {
        ESipResCode code = update_xml_result(xml_result);
        ::xml_free_manscdp_result(xml_result);            
        return code;
    }

    return SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
}

// deal sdp data
ESipResCode CGbServer::deal_sdp(void *buf, int len)
{
    SDP::sdp_parse((const char *)buf, len, &sdp_info, &media_desc, 1);

    return SIP_RESPONSE_200_OK;
}

/// deal remote BYE request 
ESipResCode CGbServer::deal_rmt_bye(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    MN::CMediaInvite    invite_param;

    /// 查找会话
    if (false == catalog_session->find_rmt_bye_dialog(msg, invite_param.transsmit_id, true))
    {
        return SIP_RESPONSE_400_BAD_REQ;
    }

    CLog::log(CLog::CLOG_LEVEL_GB_TASK, "remote bye dialog and find,transsmit id:%d\n", invite_param.transsmit_id);
    app_cb(MN_GB_RMT_BYE_MEDIA, (void *)&invite_param, sizeof(invite_param));

    return SIP_RESPONSE_200_OK;
}

/// 处理对端媒体请求
void CGbServer::deal_rmt_ack(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    MN::CMediaInvite    invite_param;

    /// 查找会话
    if (false == catalog_session->find_rmt_ack_dialog(msg, invite_param.transsmit_id))
    {
        return;
    }

    CLog::log(CLog::CLOG_LEVEL_GB_TASK, "remote ack dialog and find,transsmit id:%d\n", invite_param.transsmit_id);
    app_cb(MN_GB_RMT_ACK_MEDIA, (void *)&invite_param, sizeof(invite_param));
}

/// deal remote REGISTER request
ESipResCode CGbServer::deal_rmt_register(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time, bool authed)
{
    ESipResCode code = SIP_RESPONSE_401_UNAUTHORIZED;
    if (false == authed)
    {
        return code;
    }

    /// check authorization info
    code = SIP_RESPONSE_406_NOT_ACCEPTABLE;
    if (false == CGbUnauthReg::check_alg(msg))
    {
        CLog::log(CLog::CLOG_LEVEL_API, "md5 check failed,uri:%s\n", msg->req_uri->username);
        return code;
    }

    /// get expire time
    code = SIP_RESPONSE_485_AMBIGUOUS;
    osip_header_t *expire;
    int ret = osip_message_header_get_byname(msg, "Expires", 0, &expire);
    if ((OSIP_UNDEFINED_ERROR == ret) || (NULL == expire) || (NULL == expire->hvalue))
    {
        return code;
    }

    /// get register period
    long int reg_period;
    if (false == CStr2Digit::get_lint(expire->hvalue, reg_period))
    {
        return code;
    }

    /// reg_period is 0 when log out
    /// something to do

    if (true == reg_session->add_auth_reg(msg, ip, port, time))
    {
        code = SIP_RESPONSE_200_OK;
    }

    return code;
}

/// 处理对端媒体请求
ESipResCode CGbServer::deal_rmt_invite(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    osip_message_t *clone_msg ;
    int ret = osip_message_clone(msg, &clone_msg);
    if (OSIP_SUCCESS != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "rmt invite, osip message clone failed\n");
        return SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
    }

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

    SDP::sdp_parse(body, body_len, &sdp_info, &media_desc, 1);

    MN::CMediaInvite    invite_param;
    strncpy(invite_param.device_id, msg->req_uri->username, sizeof(invite_param.device_id));
    invite_param.ext_data       = clone_msg;
    invite_param.dev_ip         = ip;
    invite_param.dev_port       = port;
    invite_param.ssrc           = media_desc.ssrc;
    invite_param.payload_type   = media_desc.p_type;

    invite_param.rmt_rtp_addr.rtp_ip    = sdp_info.c_ip;
    invite_param.rmt_rtp_addr.rtp_port  = media_desc.port;

    ret = catalog_session->add_rmt_invite_dialog(msg, ip, port, time, invite_param.dev_id, invite_param.item_id, invite_param.dlg_id);
    if (-ENOENT == ret)
    {
        return SIP_RESPONSE_400_BAD_REQ;
    }
    else if (-EBUSY == ret)
    {
        return SIP_RESPONSE_200_OK;
    }
    else if (ret < 0)
    {
        return SIP_RESPONSE_503_SERVEICE_UNAVAILABEL;
    }

    ESipResCode code = app_cb(MN_GB_RMT_INVITE_MEDIA, (void *)&invite_param, sizeof(invite_param));

    /// free body resource
    osip_free(body);

    return code;
}


/*
 * deal event callback from sip stack
 */
ESipResCode CGbServer::sip_event_deal(void *context, int type, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time)
{
    ESipResCode code = SIP_RESPONSE_500_SERVER_INTERNAL_ERR;
    CGbServer *server = static_cast<CGbServer *>(context);

    server->osip_rmt_ip     = ip;
    server->osip_rmt_port   = port;
    switch(type)
    {
        case CGbExosip::EV_TYPE_DATA_MANSCDPXML:
            code = server->deal_xml(buf, len);
            break;

        case CGbExosip::EV_TYPE_DATA_SDP:
            code = server->deal_sdp(buf, len);
            break;

        case CGbExosip::EV_TYPE_REQ_REGISTER:
            code = server->app_cb(MN_GB_DEV_REGISTERED, buf, len);
            break;

        case CGbExosip::EV_TYPE_REQ_INVITE:
            code = server->deal_rmt_invite((osip_message_t *)buf, ip, port, time);
            break;

        case CGbExosip::EV_TYPE_AUTH_REG:
            code = server->deal_rmt_register((osip_message_t *)buf, ip, port, time, true);
            break;

        case CGbExosip::EV_TYPE_UNAUTH_REG:
            code = server->deal_rmt_register((osip_message_t *)buf, ip, port, time, false);
            break;

        case CGbExosip::EV_TYPE_MSG_MANSCDPXML:
            code = server->deal_manscdp_msg((osip_message_t *)buf, ip, port, time);
            break;

        case CGbExosip::EV_TYPE_REQ_ACK:
            server->deal_rmt_ack((osip_message_t *)buf, ip, port, time);
            break;

        case CGbExosip::EV_TYPE_REQ_BYE:
            code = server->deal_rmt_bye((osip_message_t *)buf, ip, port, time);
            break;

        default:
            CLog::log(CLog::CLOG_LEVEL_GB_TASK, "sip event deal, unrecognized type:%d\n", type);
            break;
    }

    return code;
}

/// callback event
int CGbServer::call_back(int type, void *var, int var_len)
{
    if (NULL != ev_cb)
    {
        return ev_cb(ev_cb_context, type, var, var_len);
    }

    return -1;
}

int CGbServer::reponse_catalog(gb_handle_t handle, MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len)
{
    CGbServer *server = static_cast<CGbServer *>(handle);
    return server->reponse_catalog(query_catalog_param, catalog_buf, catalog_buf_len);
}


int CGbServer::reponse_catalog(MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len)
{
    return exosip->reponse_catalog(query_catalog_param, catalog_buf, catalog_buf_len);
}

int CGbServer::rmt_invite(gb_handle_t handle, MN::CMediaInvite &invite_param)
{
    CGbServer *server = static_cast<CGbServer *>(handle);
    return server->rmt_invite(invite_param);
}

int CGbServer::rmt_invite(MN::CMediaInvite &invite_param)
{
    return exosip->rmt_invite(invite_param);
}


/// 构造函数
CGbServer::CGbServer()
{
    last_trigger_time   = 0;
    snprintf(msg_center_addr, sizeof(msg_center_addr), "%s", "/var/run/vsserver_msgcenter_0");
}

/// 析构函数
CGbServer::~CGbServer()
{
}

/// 国标链路数据回调函数
void CGbServer::link_svr_data_cb(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t time)
{
    CGbServer   *server         = static_cast<CGbServer *>(context);
    server->cur_wall_clk_time   = time;
    server->exosip->data_in(buf, buf_len, rmt_ip, rmt_port, time);
}

/// 外部通信链路数据回调函数
void CGbServer::link_exteranl_data_cb(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t time)
{
}



/// link ncproxy msg data callback
void CGbServer::link_ncproxy_data_cb(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t time)
{
    CGbServer   *server         = static_cast<CGbServer *>(context);
    server->cur_wall_clk_time   = time; 
    server->exosip->data_in(buf, buf_len, rmt_ip, rmt_port, time);
}


/// link inter msg data callback
void CGbServer::link_inter_msg_cb(void *context, void *buf, int buf_len, char *rmt_addr, uint64_t time)
{
    CGbServer   *server         = static_cast<CGbServer *>(context);
    server->cur_wall_clk_time   = time;

    server->deal_inter_msg(buf, buf_len, rmt_addr, time);
}

/// deal income msg
void CGbServer::deal_inter_msg(void *recv_data, int recv_data_len, char *rmt_addr, uint64_t cur_time)
{
    int ret = 0;

    MN::CMsgFormat          recv_msg;

    // check msg len 
    if (recv_data_len < (int)sizeof(recv_msg))
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gb adp,invalid msg len:%d\n", recv_data_len);
        return;
    }

    // check data len
    memcpy(&recv_msg, recv_data, sizeof(recv_msg));
    if (recv_msg.var_len + (int)sizeof(recv_msg) != recv_data_len)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "mc,invalid msg len:%d, var len:%d\n", recv_data_len, recv_msg.var_len);
        return;
    }

    void *var       = (uint8_t *)recv_data + sizeof(recv_msg);

    CLog::log(CLog::CLOG_LEVEL_API, "[GB]deal income msg, msg_type:%d\n", recv_msg.type);
    switch (recv_msg.type)
    {
        case MN_GB_INVITE_MEDIA:
            {
                MN::CMediaInvite invite_param;
                if (recv_msg.var_len != sizeof(invite_param))
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&invite_param, var, sizeof(invite_param));

                    CLog::log(CLog::CLOG_LEVEL_API, "[GB] invite param,dev id:%s,ssrc:%u,ip:%x,port:%u\n",
                                invite_param.device_id, invite_param.ssrc, invite_param.dev_ip, invite_param.dev_port);
                    ret = exosip->invite(invite_param);

                    /// 加入到队列
                    if (0 != ret)
                    {

                    }
                }
            }
            break;

        case MN_GB_LOCAL_RES_CATALOG:
            {
                MN::CRmtQueryCatalog query_catalog_param;
                if ( (recv_msg.var_len < (int)sizeof(query_catalog_param))
                     || (((recv_msg.var_len - sizeof(query_catalog_param)) % sizeof(MN::CCatalogItem)) != 0) )
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&query_catalog_param, var, sizeof(query_catalog_param));

                    ret = exosip->reponse_catalog(query_catalog_param, (uint8_t *)var + sizeof(MN::CRmtQueryCatalog),
                                recv_msg.var_len - sizeof(query_catalog_param));
                }
            }
            break;

        case MN_GB_RMT_INVITE_MEDIA:
            {
                MN::CMediaInvite invite_param;
                if (recv_msg.var_len != sizeof(invite_param))
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&invite_param, var, sizeof(invite_param));
                    osip_message_t *res = NULL;
                    ret = exosip->rmt_invite(invite_param, &res);

                    /// add to dialog queue
                    if (0 == ret)
                    {
                        catalog_session->add_rmt_invite_dialog(invite_param.item_id, invite_param.dlg_id, invite_param.transsmit_id, res);
                        osip_message_free(res);
                    }
                }
            }
            break;

        case MN_GB_LOCAL_QUERY_RECORDINFO:
            {
                MN::CMsgGbQueryRecordInfo rec_param;
                if (recv_msg.var_len != sizeof(rec_param))
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&rec_param, var, sizeof(rec_param));
                    char start_tm[32] = {0}, end_tm[32] = {0};
                    struct tm v_time1, v_time2;
                    localtime_r(&rec_param.start_time, &v_time1);
                    localtime_r(&rec_param.end_time, &v_time2);
                    snprintf(start_tm, sizeof(start_tm), "%04d-%02d-%02dT%02d:%02d:%02d", v_time1.tm_year + 1900, v_time1.tm_mon + 1,
                                v_time1.tm_mday, v_time1.tm_hour, v_time1.tm_min, v_time1.tm_sec);
                    snprintf(end_tm, sizeof(end_tm), "%04d-%02d-%02dT%02d:%02d:%02d", v_time2.tm_year + 1900, v_time2.tm_mon + 1,
                                v_time2.tm_mday, v_time2.tm_hour, v_time2.tm_min, v_time2.tm_sec);

                    ret = exosip->query_record_info(rec_param.device_id, rec_param.catalog_id, start_tm, end_tm,
                                rec_param.ctx, rec_param.rmt_ip, rec_param.rmt_port);
                }
            }
            break;

        case MN_GB_LOCAL_RES_RECORDINFO:
            {
                MN::CMsgGbQueryRecordInfo gb_rec_req_info;
                MN::CMsgGbResRecordInfo gb_rec_res_info; 

                int head_info_len = sizeof(gb_rec_res_info) + sizeof(gb_rec_req_info);

                if ( (recv_msg.var_len < head_info_len)
                     || (((recv_msg.var_len - head_info_len) % sizeof(MN::CMsgGbRecordItem)) != 0) )
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&gb_rec_res_info, var, sizeof(gb_rec_res_info));
                    memcpy(&gb_rec_req_info, (char *)var + sizeof(gb_rec_res_info), sizeof(gb_rec_req_info));

                    ret = exosip->reponse_record_info(gb_rec_res_info, gb_rec_req_info, (uint8_t *)var + head_info_len,
                                recv_msg.var_len - head_info_len);
                }
            }
            break;

        default:
            break;
    }

    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "++++++++++++++++++++++++gb adp deal msg failed:%d, type:%d\n", ret, recv_msg.type);
    }
}

/// link trigger
void CGbServer::link_trigger_cb(void *context, uint64_t cur_time)
{
    CGbServer   *server         = static_cast<CGbServer *>(context);

    /// 500ms 调度一次
    if (server->last_trigger_time + 500 < cur_time)
    {
        server->last_trigger_time   = cur_time;
    }
    else
    {
        return;
    }


    /// 调度未认证注册设备队列

    /// 调度认证注册设备队列
    server->reg_session->schedule(server, cur_time);

    /// 调度注册资源条目队列
    server->catalog_session->shedule(cur_time);

}

/// 发送函数
int CGbServer::link_svr_buf_send(void *context, void *buf, int len, uint32_t ip, uint16_t port, uint64_t time)
{
    CGbServer   *server         = static_cast<CGbServer *>(context);
    return server->linker->svr_send_data(buf, len, ip, port);
}

/// 发送函数回调
int CGbServer::link_svr_msg_send(void *context, void *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    // CGbServer   *server         = static_cast<CGbServer *>(context);
    return 0;
}


/// CGbServer *CGbServer::server_list           = NULL;
void CGbServer::show_server()
{
    DEBUG::dbg_printf("[GBSERVER]-------------------start show server list-------------------\n");
    CGbServer *svr = server_list;
    while (svr)
    {
        DEBUG::dbg_printf("");
        DEBUG::dbg_printf("\n");
        svr = svr->next;
    }
    DEBUG::dbg_printf("[GBSERVER]-------------------end show server list-------------------\n");
}

void CGbServer::show_dlg()
{
    CGbServer *svr = server_list;
    svr->catalog_session->show_dlg(DEBUG::dbg_printf);
}

/// GB服务器组件初始化
int CGbServer::init()
{
    int ret = osip_init(&osip_handle);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_GB_TASK, "osip init failed\n");
        return -1;
    }

    /// register cmd
    ret = DEBUG::debug_reg_cmd("gbsshow", (void *)CGbServer::show_server, "show all gb server");
    if (0 != ret)
    {
        printf("gb server reg cmd failed,ret:%d", ret);
    }

    /// register cmd
    ret = DEBUG::debug_reg_cmd("gbdlg", (void *)CGbServer::show_dlg, "show all gb dlg");
    if (0 != ret)
    {
        printf("gb server reg cmd failed,ret:%d", ret);
    }

    return 0;
}

/// 查询目录结构
int CGbServer::query_catalog(osip_message_t *msg, uint32_t ip, uint16_t port)
{
    return exosip->query_catalog(msg->from->url->username, ip, port);
}

osip_t    *CGbServer::osip_handle = NULL;

