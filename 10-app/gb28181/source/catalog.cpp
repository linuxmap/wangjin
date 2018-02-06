#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <pthread.h>

#include "common.h"
#include "xmlparser.h"
#include "gb.h"
#include "catalog.h"
#include "dialog.h"

/// 构造函数
CGbCatalog::CGbCatalog()
{
    max_item_num    = 1024;
    cur_item_num    = 0;
}

/// 析构函数
CGbCatalog::~CGbCatalog()
{
    max_item_num    = 1024;
    cur_item_num    = 0;

    max_dialog_num  = 0;
}

/// 最大注册资源条目数量
int CGbCatalog::create(uint32_t num)
{
    max_item_num    = num;      /// 最大注册设备数量

    CBB_SET_LIMIT(max_item_num, 4, 1024);
    max_dialog_num  = max_item_num;

    try
    {
        catalog_item_queue  = new CNode [max_item_num];
        dlg_queue           = new CDlgQueue;
    }
    catch (...)
    {
        return -ENOMEM;
    }

    int ret = dlg_queue->create(max_dialog_num);
    if (0 != ret)
    {
        return -ENOMEM;
    }

    cur_item_num    = 0;

    return 0;
}

/// add rec query request
int CGbCatalog::add_rmt_query_record_info(GB::CGbRecordItem &query_info, uint32_t ip, uint16_t port, uint64_t time)
{
    /// find specified item
    uint32_t index;
    if (false == item_exist(query_info.device_id, index))
    {
        return -ENOENT;
    }
    
    /// add to msg queue
    

    return 0;
}

/// 增加媒体请求应答
int CGbCatalog::add_rmt_invite_dialog(uint32_t item_id, uint32_t dlg_id, char *to_tag)
{
    if (item_id >= max_item_num)
    {
        return -EINVAL;
    }

    if (false == catalog_item_queue[item_id].used)
    {
        return -EINVAL;
    }

    CDialog *dlg = dlg_queue->get_dialog_by_id(dlg_id);
    if (NULL == dlg)
    {
        return -EINVAL;
    }

    dlg->add_local_tag(to_tag);

    return 0;
}

/// 增加媒体请求应答
int CGbCatalog::add_rmt_invite_dialog(uint32_t item_id, uint32_t dlg_id, uint32_t transmitter_id, osip_message_t *msg)
{
    if (item_id >= max_item_num)
    {
        return -EINVAL;
    }

    if (false == catalog_item_queue[item_id].used)
    {
        return -EINVAL;
    }

    CDialog *dlg = dlg_queue->get_dialog_by_id(dlg_id);
    if (NULL == dlg)
    {
        return -EINVAL;
    }

    osip_generic_param_t *tag_param_remote;
    int i = osip_from_get_tag (msg->to, &tag_param_remote);
    if (0 != i)
    {
        return -EINVAL;
    }

    dlg->add_local_tag(tag_param_remote->gvalue);
    dlg->transmitter_id = transmitter_id;

    return 0;
}

/// 查找会话
bool CGbCatalog::find_rmt_bye_dialog(osip_message_t *msg, uint32_t &transmitter_id, bool giveback)
{
    /// find specified item
    uint32_t index;
    if (false == item_exist(msg->req_uri->username, index))
    {
        return false;
    }

    CDialog *dlg = catalog_item_queue[index].rmt_caller;
    CDialog *dlg_pre = catalog_item_queue[index].rmt_caller;
    while(dlg)
    {
        if (true == dlg->ack_match_as_uas(msg))
        {
            transmitter_id  = dlg->transmitter_id;

            if (false == giveback)
            {
                return true;
            }

            if (dlg == catalog_item_queue[index].rmt_caller)
            {
                catalog_item_queue[index].rmt_caller    = dlg->next;
            }
            else
            {
                dlg_pre->next   = dlg->next;
            }

            /// free dialog
            dlg_queue->push(dlg->dlg_id);        /// give back one dlg

            return true;
        }

        dlg_pre = dlg;
        dlg     = dlg->next;
    }

    return false;
}

/// 增加媒体请求应答
bool CGbCatalog::find_rmt_ack_dialog(osip_message_t *msg, uint32_t &transmitter_id)
{
    /// find specified item
    uint32_t index;
    if (false == item_exist(msg->req_uri->username, index))
    {
        return false;
    }

    CDialog *dlg = catalog_item_queue[index].rmt_caller;
    while(dlg)
    {
        if (true == dlg->ack_match_as_uas(msg))
        {
            transmitter_id  = dlg->transmitter_id;
            return true;
        }

        dlg = dlg->next;
    }

    return false;
}

/// 增加媒体请求应答
int CGbCatalog::add_rmt_invite_dialog(osip_message_t *msg, uint32_t rmt_tl_ip, uint16_t rmt_tl_port, uint64_t time,
            uint32_t &dev_id, uint32_t &item_id, uint32_t &dlg_id)
{
    /// find specified item
    uint32_t index;
    if (false == item_exist(msg->req_uri->username, index))
    {
        return -ENOENT;
    }

    CDialog *dlg = catalog_item_queue[index].rmt_caller;
    while(dlg)
    {
        if (true == dlg->invite_match_as_uas(msg))
        {
            /// update
            dlg->invite_update_as_uas(msg, rmt_tl_ip, rmt_tl_port, time);
            return -EBUSY;
        }

        dlg = dlg->next;
    }

    CDialog *dlg_free = dlg_queue->pop();
    if (NULL == dlg_free)
    {
        return -EAGAIN;
    }

    if (false == dlg_free->invite_add_as_uac(msg, rmt_tl_ip, rmt_tl_port, time, index))
    {
        return -ENOMEM;
    }

    /// insert to list
    dlg_free->next      = catalog_item_queue[index].rmt_caller;
    catalog_item_queue[index].rmt_caller    = dlg_free;
    dlg_id              = dlg_free->dlg_id;
    item_id             = index;
    dev_id              = catalog_item_queue[index].parent_dev_id;
    
    return 0;
}

/// 条目是否存在
bool CGbCatalog::item_exist(char *id, uint32_t &item_index)
{
    uint32_t i = 0;
    for (; i < max_item_num; i++)
    {
        /// 判断是否已经存在
        if (true == catalog_item_queue[i].used)
        {
            if (0 == strcmp(id, catalog_item_queue[i].sip_id))
            {
                /// update
                item_index  = i;
                return true;
            }
        }
    }
    
    return false;
}

/// 增加条目
int CGbCatalog::add_res_item(TQueryCatalogItem &item, uint32_t parent_id, uint32_t ip, uint16_t port, uint64_t time)
{
    uint32_t i = 0;
    for (; i < max_item_num; i++)
    {
        /// 判断是否已经存在
        if (true == catalog_item_queue[i].used)
        {
            if (0 == strcmp(item.device_id, catalog_item_queue[i].sip_id))
            {
                /// update
                return update(i, item, parent_id, ip, port, time);
            }
        }
    }

    /// 执行插入操作
    return _add(item, parent_id, ip, port, time);
}

/// 增加条目
int CGbCatalog::_add(TQueryCatalogItem &item, uint32_t parent_id, uint32_t ip, uint16_t port, uint64_t time)
{
    uint32_t i = 0;
    for (; i < max_item_num; i++)
    {
        /// 判断是否已经存在
        if (false == catalog_item_queue[i].used)
        {
            break;
        }
    }

    if (max_item_num == i)
    {
        return -EAGAIN;
    }

    snprintf(catalog_item_queue[i].sip_id, sizeof(catalog_item_queue[i].sip_id), "%s", item.device_id);
    snprintf(catalog_item_queue[i].name, sizeof(catalog_item_queue[i].name), "%s", item.name);
    snprintf(catalog_item_queue[i].manufacturer, sizeof(catalog_item_queue[i].manufacturer), "%s", item.manufacturer);
    snprintf(catalog_item_queue[i].model, sizeof(catalog_item_queue[i].model), "%s", item.model);
    snprintf(catalog_item_queue[i].owner, sizeof(catalog_item_queue[i].owner), "%s", item.owner);
    snprintf(catalog_item_queue[i].civil_code, sizeof(catalog_item_queue[i].civil_code), "%s", item.civil_code);
    snprintf(catalog_item_queue[i].address, sizeof(catalog_item_queue[i].address), "%s", item.address);
    catalog_item_queue[i].parental      = item.parental;
    catalog_item_queue[i].safe_way      = item.safe_way;
    catalog_item_queue[i].register_way  = item.register_way;
    catalog_item_queue[i].secrecy       = item.secrecy;
    catalog_item_queue[i].status        = item.status;

    catalog_item_queue[i].used          = true;
    catalog_item_queue[i].parent_dev_id = parent_id;
    catalog_item_queue[i].item_id       = i;

    catalog_item_queue[i].last_update_time  = time;
    catalog_item_queue[i].dev_ip            = ip;
    catalog_item_queue[i].dev_port          = port;

    catalog_item_queue[i].local_caller      = NULL;
    catalog_item_queue[i].rmt_caller        = NULL;
    catalog_item_queue[i].next              = NULL;

    return 0;
}

/// 刷新条目信息
int CGbCatalog::update(uint32_t item_id, TQueryCatalogItem &item, uint32_t parent_id, uint32_t ip, uint16_t port, uint64_t time)
{
    catalog_item_queue[item_id].status              = item.status;
    catalog_item_queue[item_id].last_update_time    = time;

    return 0;
}

/// 构造函数
CGbCatalog::CNode::CNode()
{
    local_caller    = NULL;
    rmt_caller      = NULL;  
    next            = NULL;
    used            = false;
}

/// 析构函数
CGbCatalog::CNode::~CNode()
{
    free_resource();
}

/// 释放资源
void CGbCatalog::CNode::free_resource()
{
    parental        = 0;
    safe_way        = 0;
    register_way    = 0;
    secrecy         = 0;
    status          = 0;
    last_update_time    = 0;
    dev_ip              = 0;             /// 对端通信IP,网络序
    dev_port            = 0;           /// 对端通信端口,网络序

    used            = false;
}



/// 调度
void CGbCatalog::shedule(uint64_t cur_time)
{
#if 0
    std::map<std::string, CNode*>::iterator it = catalog_item_queue.begin();
    for (; catalog_item_queue.end() != it; it++)
    {
        it->second->shedule(cur_time);
    }
#endif
}

/// 调度
void CGbCatalog::CNode::shedule(uint64_t cur_time)
{
    /// 保活时间检查
    //...
    //...

    /// 会话检查
    CDialog *dlg    = rmt_caller;
    while(dlg)
    {
        /// 暂定2s执行一次媒体应答
        if ((EDIALOG_STATE_INVITING == dlg->state) && (cur_time - dlg->invite_time > 2000))
        {
            /// 10s超时
            if (cur_time - dlg->start_time > 10000)
            {
                /// need to do something
                /// delete current dialog

                continue;
            }


            /// 执行应答INVITE操作
            ///
        }

        dlg = dlg->next;
    }


}

    /// add rec query request
    int add_rmt_query_record_info(GB::CGbRecordItem &query_info, uint32_t ip, uint16_t port, uint64_t time);

/// 记录所有会话
void CGbCatalog::show_dlg(int (*log)(const char* format, ...))
{
    uint32_t i = 0;
    CDialog *dlg;
    for (; i < max_item_num; i++)
    {
        /// 判断是否已经存在
        if ( (true == catalog_item_queue[i].used) && (catalog_item_queue[i].rmt_caller) )
        {
            dlg = catalog_item_queue[i].rmt_caller;
            while(dlg)
            {
                log("uas item id:%s\n", catalog_item_queue[i].sip_id);
                log("dlg state:%u, item id:%u, dialog id:%u\n", dlg->state, dlg->item_id, dlg->dlg_id);
                log("call id:%s, local tag:%s, remote tag:%s\n", dlg->call_id, dlg->local_tag, dlg->remote_tag);
                log("local seq:%u, remote seq:%u\n", dlg->local_cseq, dlg->remote_cseq);
                log("invite time:%lu, start time:%lu\n", dlg->invite_time, dlg->start_time);
                log("remote tl_ip:%x, remote tl_port:%u\n", dlg->rmt_tl_ip, dlg->rmt_tl_port);
                dlg = dlg->next;
            }
        }
    }
}

