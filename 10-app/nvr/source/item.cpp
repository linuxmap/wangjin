
#include "stdlib.h"
#include "stdio.h"
#include <unistd.h>
#include <string.h>
#include <sys/prctl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>


#include "debug.h"
#include "oscbb.h"
#include "dev.h"
#include "item.h"
#include "mc.h"
#include "item_rec.h"



#define dev_log(fmt, ...) printf("[DEV] " fmt, ##__VA_ARGS__)

/// DEV namespace
namespace DEV 
{

/// max catalog item num
static CResouceItem *catalog_item_table                                 = NULL;

/// 媒体发送列表
CResouceItem::CMediaTransmitter *CResouceItem::transmitter_table_unused = NULL;      
CResouceItem::CMediaTransmitter *CResouceItem::transmitter_table_start  = NULL;      


/// 本地媒体接收起始端口
uint16_t CResouceItem::LOCAL_MEDIA_RECV_BASE_PORT                       = 10000;

/// 本地媒体接收地址
uint32_t CResouceItem::LOCAL_MEDIA_RECV_ADDR;

/// global data define
uint32_t CResouceItem::valid_item_num           = 0;
uint32_t CResouceItem::max_item_num             = 128;
uint32_t CResouceItem::valid_tramsmitter_num    = 256;
char CResouceItem::send_buf[8192];
int  CResouceItem::send_buf_len = 0;

/// log handler set
int (*CResouceItem::log)(const char* format, ...)   = DEBUG::dbg_printf;

/// asynchronous msg queue define
uint32_t CResouceItem::async_msg_ctx    = 0;
CDevAsyncMsg *CResouceItem::async_msg_queue = NULL;
uint32_t CResouceItem::async_msg_size   = 0;

uint32_t CResouceItem::get_async_msg_ctx()
{
    CResouceItem::async_msg_ctx++;
    if (CResouceItem::async_msg_ctx == 0x7FFFFFFF)
    {
        CResouceItem::async_msg_ctx = 1;
    }

repeat:
    CDevAsyncMsg *async_msg = async_msg_queue;
    while (async_msg)
    {
        if (async_msg->ctx == CResouceItem::async_msg_ctx)
        {
            CResouceItem::async_msg_ctx++;
            goto repeat;
        }
        async_msg   = async_msg->next;
    }

    return CResouceItem::async_msg_ctx;
}

/// 初始化
int CResouceItem::init(uint32_t max, uint32_t media_recv_ip)
{
    max_item_num    = max;
    
    CBB_SET_LIMIT(max_item_num, 1, 1024);
    valid_tramsmitter_num   = 2 * max_item_num;

    try
    {
        catalog_item_table          = new CResouceItem [max_item_num];
        transmitter_table_start     = new CMediaTransmitter [valid_tramsmitter_num];
    }
    catch (...)
    {
        dev_log("allocate memory failed,size of catalog item:%d\n", max_item_num);
        return -ENOMEM;
    }

    transmitter_table_unused    = transmitter_table_start; 

    transmitter_table_unused->init(transmitter_table_unused, valid_tramsmitter_num);

    LOCAL_MEDIA_RECV_ADDR   = media_recv_ip;

    return 0;
}

void CResouceItem::deal_rec_list(MN::CMsgGbQueryRecordInfo &rec_req_info, MN::CMsgGbResRecordInfo &rec_res_info, CDevRecQueryResult *result)
{
    if (!rec_req_info.rmt_ip)
    {
        return;
    }

    MN::CMsgFormat msg;
    msg.type        = MN_GB_LOCAL_RES_RECORDINFO;   // msg type
    msg.sn          = 0;

    CDevRecordItem  *item   = result->rec_list;          /// record list item

    MN::CMsgGbRecordItem gb_rec_item;
    MN::CMsgGbResRecordInfo gb_rec_info; 

    strncpy(gb_rec_info.device_id, rec_res_info.device_id, sizeof(gb_rec_info.device_id));
    strncpy(gb_rec_info.name, rec_res_info.name, sizeof(gb_rec_info.name));
    gb_rec_info.sum_num = result->rec_sum_num;
    gb_rec_info.ctx     = rec_req_info.ctx;

    int list_num     = 0, ret;
    send_buf_len     = sizeof(gb_rec_info) + sizeof(rec_req_info);
    memcpy(send_buf + sizeof(gb_rec_info), &rec_req_info, sizeof(rec_req_info));
    while(item)
    {
        memcpy(gb_rec_item.device_id, item->device_id, sizeof(gb_rec_item.device_id));
        memcpy(gb_rec_item.name, item->name, sizeof(gb_rec_item.name));
        memcpy(gb_rec_item.file_path, item->file_path, sizeof(gb_rec_item.file_path));
        memcpy(gb_rec_item.address, item->address, sizeof(gb_rec_item.address));
        memcpy(gb_rec_item.recorder_id, item->recorder_id, sizeof(gb_rec_item.recorder_id));
        gb_rec_item.start_time  = item->start_time;
        gb_rec_item.end_time    = item->end_time;
        gb_rec_item.secrecy     = item->secrecy;
        gb_rec_item.type        = item->type;
        list_num++;
        memcpy(send_buf + send_buf_len, &gb_rec_item, sizeof(gb_rec_item));
        send_buf_len             += sizeof(gb_rec_item);

        /// callback
        if (4 == list_num)
        {
            gb_rec_info.list_num = list_num;
            memcpy(send_buf, &gb_rec_info, sizeof(gb_rec_info));
            msg.var_len = send_buf_len;

            ret = MC::mc_send_to_gb(&msg, sizeof(msg), send_buf, send_buf_len);
            dev_log("[dev] reply record item to gbserver list num:%d, sum num:%d, ret:%d\n", list_num, rec_res_info.sum_num, ret);

            list_num             = 0;
            send_buf_len     = sizeof(gb_rec_info) + sizeof(rec_req_info);
        }

        item    = item->next;
    }

    /// callback
    if ( (0 != list_num) || (0 == rec_res_info.sum_num) )
    {
        gb_rec_info.list_num = list_num;
        memcpy(send_buf, &gb_rec_info, sizeof(gb_rec_info));
        msg.var_len = send_buf_len;

        ret = MC::mc_send_to_gb(&msg, sizeof(msg), send_buf, send_buf_len);
        dev_log("[dev] last reply record item to gbserver list num:%d, sum num:%d, ret:%d\n", list_num, rec_res_info.sum_num, ret);
    }
}

/*
 * on success, return zero,on error, return nonzero
 */
int CResouceItem::get_item_by_id(char *dst_id, uint32_t *index)
{
    for (uint32_t i = 0; i < max_item_num; i++)
    {
        if ( (true == catalog_item_table[i].valid) && (0 == catalog_item_table[i].device_id.compare(dst_id)) )
        {
            /// find it
            if (index)
            {
                *index   = i;
            }
            return 0;
        }
    }
    
    dev_log("not found sip id:%s\n", dst_id);
    return -ENOENT;
}

/*
 * on success, return zero,on error, return nonzero
 */
int CResouceItem::get_async_msg_by_ctx(uint32_t ctx, CDevAsyncMsg **dst_msg)
{
    /// lookfor msg
    CDevAsyncMsg *async_msg = async_msg_queue;
    while (async_msg)
    {
        if (async_msg->ctx == ctx)
        {
            if (dst_msg)
            {
                *dst_msg    = async_msg;
            }
            return 0;
        }
        async_msg   = async_msg->next;
    }

    /// not found user-context
    return -ENOENT;
}

/*
 * add rec item to result list
 */
int CResouceItem::add_rec_item_to_result(CDevRecQueryResult *result, MN::CMsgGbRecordItem *rec_item, int num, uint64_t time)
{
    CDevRecordItem *list;
    for (int i = 0; i < num; i++)
    {
        list    = result->rec_list;
        while (list)
        {
            if (true == list->is_same(*rec_item, time))
            {
                dev_log("rec item add same,file path:%s, time range:[%ld, %ld]\n",
                            rec_item->file_path, rec_item->start_time, rec_item->end_time);
                return 0;
            }
            else
            {
                list = list->next;
            }
        }

        /// add new
        list = NULL;
        if (false == CDevRecordItem::get_one_node(&list))
        {
            dev_log("get one rec item node failed,again\n");
            return -EAGAIN;
        }
        list->set_to_value(*rec_item, time);
        dev_log("add one rec item node file path:%s, time range:[%ld, %ld]\n",
                    rec_item->file_path, rec_item->start_time, rec_item->end_time);

        list->next = result->rec_list;
        result->rec_list = list;
        result->rec_cur_num++;
        rec_item++;
    }
    dev_log("current rec list size:%u\n", result->rec_cur_num);
    return 0;
}

/*
 * free aync rec query msg
 */
void CResouceItem::del_rec_info(CDevRecQueryResult *result, CDevAsyncMsg *async_msg)
{
    CDevRecordItem *list   = result->rec_list;
    while (list)
    {
        CDevRecordItem* tmp     = list->next;
        CDevRecordItem::giveback_one_node(list);
        list    = tmp;
    }

    /// free request and response
    if (async_msg == async_msg_queue)
    {
        async_msg_queue = async_msg->next;
    }
    else
    {
        CDevAsyncMsg *msg_tmp = async_msg_queue;
        while ( (msg_tmp) && (msg_tmp->next != async_msg) )
        {
            msg_tmp = msg_tmp->next;
        }

        if (msg_tmp)
        {
            msg_tmp->next   = async_msg->next;
        }
    }

    dev_free(async_msg->req_data);
    dev_free(async_msg->response);
    dev_free(async_msg);

    if (CResouceItem::async_msg_size > 0)
    {
        CResouceItem::async_msg_size--;
    }          
}

/*
 * add rec info
 */
int CResouceItem::add_rec_info(MN::CMsgGbResRecordInfo &rec_info, MN::CMsgGbRecordItem *rec_item, uint64_t time)
{
    /// specified device exist
    if (0 != get_item_by_id(rec_info.device_id))
    {
        return -ENOENT;
    }

    /// async msg exist
    CDevAsyncMsg *async_msg = NULL;
    if (0 != get_async_msg_by_ctx(rec_info.ctx, &async_msg))
    {
        return -ENOENT;
    }

    /// deal current msg
    async_msg->last_update_time = time;
    CDevRecQueryResult *result = (CDevRecQueryResult *)async_msg->response;
    if ( (0 == rec_info.sum_num) || (NULL == rec_item) )
    {
        goto dump_rec;
    }

    if (!result->rec_sum_num)
    {
        result->rec_sum_num = rec_info.sum_num;
    }
    else if (result->rec_sum_num != rec_info.sum_num)
    {
        dev_log("record list num not match,orgin:%u, now:%d\n", result->rec_sum_num, rec_info.sum_num);
        result->rec_sum_num     = CBBMIN(result->rec_sum_num, rec_info.sum_num);
        goto dump_rec;
    }

    result->update_time     = time;

    /// add to list failed,FIXME:we need to callback current node and keep async msg
    if (0 != add_rec_item_to_result(result, rec_item, rec_info.list_num, time))
    {
        goto dump_rec;
    }
    
    /// dump to dst
    if (result->rec_cur_num == result->rec_sum_num)
    {
dump_rec:
        /// async_msg->req_data
        MN::CMsgGbQueryRecordInfo rec_req_info;
        memcpy(&rec_req_info, async_msg->req_data, sizeof(rec_req_info));
        deal_rec_list(rec_req_info, rec_info, result);

        /// deal record list
        dev_log("delete record request async msg,rec item num:%d\n", result->rec_cur_num);
        del_rec_info(result, async_msg);
    }

    return 0;
}

/// remote query record info
int CResouceItem::query_record_info(MN::CMsgGbQueryRecordInfo &rec_param_in, uint64_t cur_time)
{
    int ret;
    uint32_t i;

    for (i = 0; i < max_item_num; i++)
    {
        if ( (true == catalog_item_table[i].valid)
            && (!strcmp(rec_param_in.catalog_id, catalog_item_table[i].device_id.c_str())) )
        {
            if (CResouceItem::async_msg_size > 1024)  /// max async msg num
            {
                dev_log("msg size out\n");
                return -EAGAIN;
            }

            MN::CMsgFormat msg;
            MN::CMsgGbQueryRecordInfo rec_param;

            // msg type
            msg.type        = MN_GB_LOCAL_QUERY_RECORDINFO;
            msg.sn          = 0;
            msg.var_len     = sizeof(rec_param);

            // invite param
            snprintf(rec_param.device_id, sizeof(rec_param.device_id), "%s", catalog_item_table[i].parent_device_id.c_str());
            snprintf(rec_param.catalog_id, sizeof(rec_param.catalog_id), "%s", catalog_item_table[i].device_id.c_str());
            rec_param.end_time      = rec_param_in.end_time;
            rec_param.start_time    = rec_param_in.start_time;
            rec_param.rmt_ip        = catalog_item_table[i].ip;
            rec_param.rmt_port      = catalog_item_table[i].port;
            rec_param.ctx           = get_async_msg_ctx();

            ret = MC::mc_send_to_gb(&msg, sizeof(msg), &rec_param, sizeof(rec_param));
            dev_log("query recorder info param ret:%d,catalog id:%s,ip:%x,port:%u**********************************\n",
                        ret, rec_param.catalog_id, rec_param.rmt_ip, ntohs(rec_param.rmt_port));

            CDevAsyncMsg    *async_msg = (CDevAsyncMsg    *)dev_mallocz(sizeof(*async_msg));
            if (NULL == async_msg)
            {
                dev_log("malloc async msg faile, no memory\n");
                return -ENOMEM;
            }
            async_msg->msg_type       = MN_GB_LOCAL_QUERY_RECORDINFO;
            async_msg->ctx            = rec_param.ctx;
            /// async_msg->from           =   /// TODO:set async_msg from addr
            async_msg->start_time       = cur_time;
            async_msg->last_update_time = cur_time;
            async_msg->timeout_time     = 5000;             /// 5s
            async_msg->req_len          = sizeof(rec_param_in);
            async_msg->req_data         = dev_mallocz(async_msg->req_len);
            async_msg->response         = dev_mallocz(sizeof(CDevRecQueryResult));
            if ( (NULL == async_msg->req_data) || (NULL == async_msg->response) )
            {
                dev_free(async_msg->req_data);
                dev_free(async_msg);
                dev_log("malloc async msg request and response faile, no memory\n");
                return -ENOMEM;
            }
            CDevRecQueryResult *result = (CDevRecQueryResult *)async_msg->response;
            result->start_time      = cur_time;
            result->update_time     = cur_time;

            memcpy(async_msg->req_data, &rec_param_in, sizeof(rec_param_in));
            async_msg->next       = async_msg_queue;
            async_msg_queue       = async_msg;
            CResouceItem::async_msg_size++;

            return 0;
        }
    }

    show_all_item(printf);
    dev_log("query recorder info failed,id:%s\n", rec_param_in.catalog_id);
    return -ENOENT;
}

/// opertion:query catalog item recorder infomation
void CResouceItem::query_record_info(uint64_t cur_time)
{
    int ret;
    uint32_t i;
    static int internal = 0;
    internal++;
    return;
    if ((internal & 0x7) != 0x7)
    {
        return;
    }

    for (i = 0; i < max_item_num; i++)
    {
        if ( (true == catalog_item_table[i].valid) && (true == catalog_item_table[i].on) )
        {
            if (CResouceItem::async_msg_size > 1024)  /// max async msg num
            {
                dev_log("msg size out\n");
                return;
            }

            MN::CMsgFormat msg;
            MN::CMsgGbQueryRecordInfo rec_param;

            // msg type
            msg.type        = MN_GB_LOCAL_QUERY_RECORDINFO;
            msg.sn          = 0;
            msg.var_len     = sizeof(rec_param);

            // invite param
            snprintf(rec_param.device_id, sizeof(rec_param.device_id), "%s", catalog_item_table[i].parent_device_id.c_str());
            snprintf(rec_param.catalog_id, sizeof(rec_param.catalog_id), "%s", catalog_item_table[i].device_id.c_str());
            rec_param.end_time      = time(NULL) - 3600;
            rec_param.start_time    = rec_param.end_time - 86400;
            rec_param.rmt_ip        = catalog_item_table[i].ip;
            rec_param.rmt_port      = catalog_item_table[i].port;
            rec_param.ctx           = get_async_msg_ctx();

            ret = MC::mc_send_to_gb(&msg, sizeof(msg), &rec_param, sizeof(rec_param));
            dev_log("query recorder info param ret:%d,dev id:%s,ip:%x,port:%u**********************************\n",
                        ret, rec_param.device_id, rec_param.rmt_ip, ntohs(rec_param.rmt_port));

            CDevAsyncMsg    *async_msg = (CDevAsyncMsg *)dev_mallocz(sizeof(*async_msg));
            if (NULL == async_msg)
            {
                dev_log("malloc async msg faile, no memory\n");
                return;
            }
            async_msg->msg_type       = MN_GB_LOCAL_QUERY_RECORDINFO;
            async_msg->ctx            = rec_param.ctx;
            /// async_msg->from           =   /// TODO:set async_msg from addr
            async_msg->start_time       = cur_time;
            async_msg->last_update_time = cur_time;
            async_msg->timeout_time     = 5000;             /// 5s
            async_msg->req_len          = sizeof(MN::CMsgGbQueryRecordInfo);
            async_msg->req_data         = dev_mallocz(async_msg->req_len);
            async_msg->response         = dev_mallocz(sizeof(CDevRecQueryResult));
            if ( (NULL == async_msg->req_data) || (NULL == async_msg->response) )
            {
                dev_free(async_msg->req_data);
                dev_free(async_msg);
                dev_log("malloc async msg request and response faile, no memory\n");
                return;
            }
            CDevRecQueryResult *result = (CDevRecQueryResult *)async_msg->response;
            result->start_time      = cur_time;
            result->update_time     = cur_time;

            memcpy(async_msg->req_data, &rec_param, sizeof(rec_param));
            async_msg->next       = async_msg_queue;
            async_msg_queue       = async_msg;
            CResouceItem::async_msg_size++;
        }
    }
}

/// 执行媒体请求操作
void CResouceItem::invite_media()
{
    int ret;
    uint32_t i;
    for (i = 0; i < max_item_num; i++)
    {
        if ( (true == catalog_item_table[i].valid) && (true == catalog_item_table[i].on) && (false == catalog_item_table[i].play) )
        {
            MN::CMsgFormat msg;
            MN::CMediaInvite media_invite;

            // msg type
            msg.type        = MN_GB_INVITE_MEDIA;
            msg.sn          = 0;
            msg.var_len     = sizeof(media_invite);

            // invite param
            snprintf(media_invite.device_id, sizeof(media_invite.device_id), "%s", catalog_item_table[i].device_id.c_str());
            media_invite.local_net_info = catalog_item_table[i].local_addr[0];
            media_invite.dev_ip         = catalog_item_table[i].ip;
            media_invite.dev_port       = catalog_item_table[i].port;
            media_invite.ssrc           = 100000004 + i * 10;
            catalog_item_table[i].play  = true;

            ret = MC::mc_send_to_gb(&msg, sizeof(msg), &media_invite, sizeof(media_invite));
            dev_log("[dev] invite param ret:%d,dev id:%s,ssrc:%u,ip:%x,port:%u\n",
                        ret, media_invite.device_id, media_invite.ssrc, media_invite.dev_ip, ntohs(media_invite.dev_port));
        }
    }
}

void CResouceItem::run(uint64_t cur_time)
{
    invite_media();

    query_record_info(cur_time);
}

/// add cata_log item
int CResouceItem::add(MN::CCatalogItem &cata_log)
{
    uint32_t i;

    // 检查重复
    for (i = 0; i < max_item_num; i++)
    {
        if (true == catalog_item_table[i].valid)
        {
            if (0 == catalog_item_table[i].device_id.compare(cata_log.device_id))
            {
                dev_log("resource item update:%s\n", cata_log.device_id);
                return 0;
            }
        }
    }


    for (i = 0; i < max_item_num; i++)
    {
        if (false == catalog_item_table[i].valid)
        {
            catalog_item_table[i].device_id.assign(cata_log.device_id);
            catalog_item_table[i].name.assign(cata_log.name);
            catalog_item_table[i].manufacturer.assign(cata_log.manufacturer);
            catalog_item_table[i].model.assign(cata_log.model);
            catalog_item_table[i].owner.assign(cata_log.owner);
            catalog_item_table[i].civil_code.assign(cata_log.civil_code);
            catalog_item_table[i].address.assign(cata_log.address);
            catalog_item_table[i].parent_device_id.assign(cata_log.parent_device_id);

            catalog_item_table[i].parental      = cata_log.parental;
            catalog_item_table[i].safe_way      = cata_log.safe_way;
            catalog_item_table[i].register_way  = cata_log.register_way;
            catalog_item_table[i].secrecy       = cata_log.secrecy;
            catalog_item_table[i].on            = cata_log.on;
            catalog_item_table[i].ip            = cata_log.ip;
            catalog_item_table[i].port          = cata_log.port;

            catalog_item_table[i].valid         = true; 
            catalog_item_table[i].play          = false; 
            catalog_item_table[i].channel_id    = i;

            for (int j = 0; j < 4; j++)
            {
                catalog_item_table[i].local_addr[j].rtp_port      = LOCAL_MEDIA_RECV_BASE_PORT + i * 10 + j * 2;
                catalog_item_table[i].local_addr[j].rtp_ip        = LOCAL_MEDIA_RECV_ADDR;
                catalog_item_table[i].local_addr[j].rtcp_port     = catalog_item_table[i].local_addr[j].rtp_port + 1;
                catalog_item_table[i].local_addr[j].rtcp_ip       = catalog_item_table[i].local_addr[j].rtp_ip;
            }

            valid_item_num++;

            dev_log("catalog add,index:%d, status:%d\n", i, catalog_item_table[i].on);
            break;
        }
    }

    if (max_item_num == i)
    {
        dev_log("catalog add failed\n");
        return -1;
    }

    return 0;
}


/// add cata_log item
CResouceItem::CResouceItem()
{
    valid               = false;
    play                = false;
    transmitter         = NULL;
}


int CResouceItem::query_cata_log(MN::CRmtQueryCatalog &query_catalog_param)
{
    int ret = 0;
    MN::CCatalogItem cata_log;
    MN::CMsgFormat msg;

    // msg type
    msg.type        = MN_GB_LOCAL_RES_CATALOG;
    msg.sn          = 0;

    // set query param
    query_catalog_param.num = valid_item_num;
    memcpy(send_buf + sizeof(msg), &query_catalog_param, sizeof(query_catalog_param));

    if (0 == valid_item_num)
    {
        msg.var_len             = sizeof(query_catalog_param); 
        memcpy(send_buf, &msg, sizeof(msg));

        ret = MC::mc_send_to_gb(send_buf, sizeof(msg) + msg.var_len, NULL, 0);
        if (ret != 0)
        {
            dev_log("send gb catalog response failed\n");
        }
        return ret;
    }


    // set all dev
    int valid_num = 0;
    uint32_t i = 0;
    for (; i < max_item_num; i++)
    {
        if (true == catalog_item_table[i].valid)
        {
            snprintf(cata_log.device_id, sizeof(cata_log.device_id), "%s", catalog_item_table[i].device_id.c_str());
            snprintf(cata_log.name, sizeof(cata_log.name), "%s", catalog_item_table[i].name.c_str());
            snprintf(cata_log.manufacturer, sizeof(cata_log.manufacturer), "%s", catalog_item_table[i].manufacturer.c_str());
            snprintf(cata_log.model, sizeof(cata_log.model), "%s", catalog_item_table[i].model.c_str());
            snprintf(cata_log.owner, sizeof(cata_log.owner), "%s", catalog_item_table[i].owner.c_str());
            snprintf(cata_log.civil_code, sizeof(cata_log.civil_code), "%s", catalog_item_table[i].civil_code.c_str());
            snprintf(cata_log.address, sizeof(cata_log.address), "%s", catalog_item_table[i].address.c_str());

            cata_log.parental       = catalog_item_table[i].parental;
            cata_log.safe_way       = catalog_item_table[i].safe_way;
            cata_log.register_way   = catalog_item_table[i].register_way;
            cata_log.secrecy        = catalog_item_table[i].secrecy;
            cata_log.on             = catalog_item_table[i].play; 

            memcpy(send_buf + sizeof(msg) + sizeof(query_catalog_param) + sizeof(cata_log) * (valid_num & 0x1), &cata_log, sizeof(cata_log));

            // 2 item send
            if (0 != (valid_num & 0x1))
            {
                msg.var_len             = sizeof(cata_log) * 2 + sizeof(query_catalog_param); 
                memcpy(send_buf, &msg, sizeof(msg));

                int ret = MC::mc_send_to_gb(send_buf, sizeof(msg) + msg.var_len, NULL, 0);
                if (ret != 0)
                {
                    dev_log("send gb catalog response failed\n");
                    return ret;
                }
            }
            valid_num++;
        }
    }

    if (0 == (valid_num & 0x1))
    {
        msg.var_len             = sizeof(cata_log) + sizeof(query_catalog_param); 
        memcpy(send_buf, &msg, sizeof(msg));

        ret = MC::mc_send_to_gb(send_buf, sizeof(msg) + msg.var_len, NULL, 0);
        if (ret != 0)
        {
            dev_log("send gb catalog response failed\n");
        }
    }

    return ret;
}

/// close media channel
int CResouceItem::rmt_bye_media(MN::CMediaInvite &invite_param)
{
    if (invite_param.transsmit_id >= valid_tramsmitter_num)
    {
        return -EINVAL;
    }

    CMediaTransmitter *transmitter_tmp  = transmitter_table_start + invite_param.transsmit_id;

    /// 通知vtdu模块
    MN::CMsgAddSwitchIpv4 msg2vtdu;
    msg2vtdu.channel_id     = transmitter_tmp->vtdu_ch_id; 
    msg2vtdu.to_ip          = transmitter_tmp->to_ip;
    msg2vtdu.spy_ip         = 0;
    msg2vtdu.to_port        = transmitter_tmp->to_port;
    msg2vtdu.spy_port       = 0;
    msg2vtdu.ssrc           = transmitter_tmp->ssrc;
    msg2vtdu.op_result      = 0;
    int ret = MC::mc_send_to_vtdu(MN_VTDU_DEL_SWITCH_IPV4, (void *)&msg2vtdu, sizeof(msg2vtdu), NULL, 0);
    dev_log("rmt ack media,port:%u, ret:%d\n", msg2vtdu.to_port, ret);

    /// give up one node to the free table
    transmitter_table_unused->add_one_node(&transmitter_table_unused, transmitter_tmp);

    return ret;
}

/// 远端确认媒体邀请
int CResouceItem::rmt_ack_media(MN::CMediaInvite &invite_param)
{
    if (invite_param.transsmit_id >= valid_tramsmitter_num)
    {
        return -EINVAL;
    }

    CMediaTransmitter *transmitter_tmp  = transmitter_table_start + invite_param.transsmit_id;

    /// 通知vtdu模块
    MN::CMsgAddSwitchIpv4 msg2vtdu;
    msg2vtdu.channel_id     = transmitter_tmp->vtdu_ch_id; 
    msg2vtdu.to_ip          = transmitter_tmp->to_ip;
    msg2vtdu.spy_ip         = 0;
    msg2vtdu.to_port        = transmitter_tmp->to_port;
    msg2vtdu.spy_port       = 0;
    msg2vtdu.ssrc           = transmitter_tmp->ssrc;
    msg2vtdu.op_result      = 0;
    int ret = MC::mc_send_to_vtdu(MN_VTDU_ADD_SWITCH_IPV4, (void *)&msg2vtdu, sizeof(msg2vtdu), NULL, 0);
    dev_log("rmt ack media,port:%u, ret:%d\n", msg2vtdu.to_port, ret);

    return ret;
}

/// 远端请求媒体
int CResouceItem::rmt_invite_media(MN::CMediaInvite &invite_param)
{
    int ret;
    uint32_t i;
    bool find = false;
    CMediaTransmitter *transmitter_tmp  = NULL;
    for (i = 0; i < max_item_num; i++)
    {
        if ( (true == catalog_item_table[i].valid) && (!strcmp(invite_param.device_id, catalog_item_table[i].device_id.c_str())) )
        {
            /// find specified device
            transmitter_tmp = transmitter_table_unused->get_one_node(&transmitter_table_unused);
            if (NULL != transmitter_tmp)
            {
                find = true;
                break;
            }
            
            dev_log("rmt_invite media, find result:%d\n", find);
            break;
        }
    }

    if (find)
    {
        invite_param.local_net_info         = catalog_item_table[i].local_addr[0];
        invite_param.transsmit_id           = transmitter_tmp->transsmit_id;

        transmitter_tmp->init_data();
        transmitter_tmp->vtdu_ch_id = i;
        transmitter_tmp->to_ip      = invite_param.rmt_rtp_addr.rtp_ip;
        transmitter_tmp->spy_ip     = 0;
        transmitter_tmp->to_port    = invite_param.rmt_rtp_addr.rtp_port; 
        transmitter_tmp->spy_port   = 0;
        transmitter_tmp->ssrc       = invite_param.ssrc;
        transmitter_tmp->ptype      = invite_param.payload_type;
        transmitter_tmp->start      = false;
        transmitter_tmp->last_time  = CBB::gettime_ms();            /// 上一次更新时间
        
        /// 增加一个节点
        catalog_item_table[i].transmitter->add_one_node(&catalog_item_table[i].transmitter, transmitter_tmp);
    }

    MN::CMsgFormat msg;

    // msg type
    msg.type        = MN_GB_RMT_INVITE_MEDIA;
    msg.sn          = 0;
    msg.var_len     = sizeof(invite_param);

    ret = MC::mc_send_to_gb(&msg, sizeof(msg), &invite_param, sizeof(invite_param));
    dev_log("to invite media,index:%d, ret:%d\n", i, ret);

    return ret;
}

void CResouceItem::show_all_item(int(*pf)(const char *fmt, ...))
{
    if (pf == NULL)
    {
        log("[DEV]--------------valid item num:%d--------------\n", valid_item_num);
    }
    else
    {
        pf("[DEV]--------------valid item num:%d--------------\n", valid_item_num);
    }

    for (uint32_t i =0; i < max_item_num; i++)
    {
        catalog_item_table[i].show(pf);
        if (0x8 == (i & 0xF))
        {
            CBB::cbb_delay(10);
        }
    }
}

void CResouceItem::show(int(*pf)(const char *fmt, ...))
{
    if (false == valid)
    {
        return;
    }

    if (NULL == pf)
    {
        pf = log;
    }


    pf("[DEV]valid:%d,play:%d,device id:%s,parent device id:%s\n", valid, play, device_id.c_str(), parent_device_id.c_str());
    pf("[DEV]name:%-10smanufacturer:%-10s\n", name.c_str(), manufacturer.c_str());
    pf("[DEV]model:%-10sowner:%-10s\n", model.c_str(), owner.c_str());
    pf("[DEV]civil_code:%-10saddress:%-10s\n", civil_code.c_str(), address.c_str());
    pf("[DEV]parental:%d,safe way:%d,register_way:%d,secrecy:%d,on:%d\n", parental, safe_way, register_way, secrecy, on);
}

int cata_log_add(MN::CCatalogItem &cata_log)
{
    return CResouceItem::add(cata_log);
}

int cata_log_del(MN::CCatalogItem &cata_log)
{
    //// 有待实现
    return 0;
}

int query_cata_log(MN::CRmtQueryCatalog &query_catalog_param)
{
    return CResouceItem::query_cata_log(query_catalog_param);
}

int rmt_invite_media(MN::CMediaInvite &invite_param)
{
    return CResouceItem::rmt_invite_media(invite_param);
}

int rmt_bye_media(MN::CMediaInvite &invite_param)
{
    return CResouceItem::rmt_bye_media(invite_param);
}

int add_rec_info(MN::CMsgGbResRecordInfo &rec_info, MN::CMsgGbRecordItem *rec_item, uint64_t time)
{
    return CResouceItem::add_rec_info(rec_info, rec_item, time);
}

int rmt_ack_media(MN::CMediaInvite &invite_param)
{
    return CResouceItem::rmt_ack_media(invite_param);
}

/// remote query record info
int rmt_query_record_info(MN::CMsgGbQueryRecordInfo &query_param, uint64_t time)
{
    return CResouceItem::query_record_info(query_param, time);
}

/// 构造函数
CDevCfgParam::CDevCfgParam()
{
    max_item_num    = 128;;       /// default:128
    max_dev_num     = 128;        /// default:128
    media_recv_addr = 0;
}

/// 初始化
void CResouceItem::CMediaTransmitter::init(CMediaTransmitter *list_hdr, uint32_t list_size)
{
    if (1 == list_size)
    {
        list_hdr->next = NULL;
        list_hdr[0].transsmit_id = 0;
        return;
    }

    CMediaTransmitter *tmp = list_hdr;

    uint32_t i =0;
    for (; i < list_size - 1; i++)
    {
        list_hdr[i].transsmit_id = i;
        tmp->next   = list_hdr + i + 1;
        tmp = tmp->next;
    }

    list_hdr[i].transsmit_id    = i;
    tmp->next                   = NULL;
}

/// 获取一个节点
CResouceItem::CMediaTransmitter *CResouceItem::CMediaTransmitter::get_one_node(CMediaTransmitter **list_hdr)
{
    if (NULL == *list_hdr)
    {
        return NULL;
    }

    CMediaTransmitter * node = *list_hdr;
    *list_hdr = (*list_hdr)->next;
    return node;
}

/// 增加一个节点
void CResouceItem::CMediaTransmitter::add_one_node(CResouceItem::CMediaTransmitter **list_hdr, CMediaTransmitter *node)
{
    if (NULL == *list_hdr)
    {
        *list_hdr   = node;
        node->next  = NULL;
        return;
    }

    node->next  = *list_hdr;
    *list_hdr   = node;    
}

void CResouceItem::CMediaTransmitter::init_data()
{
    to_ip               = 0;            /// 网络序
    spy_ip              = 0;            /// 网络序
    to_port             = 0;            /// 主机序
    spy_port            = 0;            /// 主机序
    ssrc                = 0;            /// 转发通道SSRC值(为0保持)
    ptype               = 0;            /// 载荷类型

    item_id             = 0;            /// 条目ID
    dev_id              = 0;            /// 设备ID
    dlg_id              = 0;            /// 会话ID
    start               = false;        /// 开始
    last_time           = 0;            /// 上一次更新时间

    last_update_time    = 0;            /// 最近一次更新时间
    snd_packets         = 0;            /// 累计发送包数
    snd_error_packets   = 0;            /// 累计发送错误包数
    snd_bytes           = 0;            /// 发送字节数
    snd_error_bytes     = 0;            /// 累计发送字节数
}

/// 媒体发送器
CResouceItem::CMediaTransmitter::CMediaTransmitter()
{
    to_ip               = 0;            /// 网络序
    spy_ip              = 0;            /// 网络序
    to_port             = 0;            /// 主机序
    spy_port            = 0;            /// 主机序
    ssrc                = 0;            /// 转发通道SSRC值(为0保持)
    ptype               = 0;            /// 载荷类型

    item_id             = 0;            /// 条目ID
    dev_id              = 0;            /// 设备ID
    dlg_id              = 0;            /// 会话ID
    transsmit_id        = 0;            /// 发送器ID
    start               = false;        /// 开始
    last_time           = 0;            /// 上一次更新时间

    last_update_time    = 0;            /// 最近一次更新时间
    snd_packets         = 0;            /// 累计发送包数
    snd_error_packets   = 0;            /// 累计发送错误包数
    snd_bytes           = 0;            /// 发送字节数
    snd_error_bytes     = 0;            /// 累计发送字节数
    next                = NULL;
}

}
