#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include "common.h"
#include "gb.h"
#include "session.h"

#include "gb_server.h"

/// 构造函数
CGbRegSession::CGbRegSession()
{
}

/// 析构函数
CGbRegSession::~CGbRegSession()
{
}

/// 最大注册设备数量
int CGbRegSession::create(uint32_t num)
{
    max_reg_dev_num = num;
    cur_reg_dev_num = 0;

    return 0;
}


/// 检查会话时间是否超时
bool CGbRegSession::check_timeout(CRegNode *node, uint64_t cur_time)
{
    if (node->last_reg_time + (node->alive_time >> 10) < cur_time)
    {
        return false;
    }

    return true;
}


/// 调度
/// 更新注册设备信息
/// 更新目录查询信息
void CGbRegSession::schedule(CGbServer *server, uint64_t time)
{
    std::map<std::string, CRegNode*>::iterator it = dev_reg_queue.begin();
    while(it != dev_reg_queue.end())
    {
        /// 检查时间
        if (false == check_timeout(it->second, time))
        {
            // CLog::log(CLog::CLOG_LEVEL_API, "gbsession dev reg timeout:%s\n",
            //             it->first.c_str());
        }

        /// 请求目录结构
        if (false == it->second->get_catalog)
        {
            int ret;
            ret = server->query_catalog(it->second->last_reg_msg, it->second->reg_ip, it->second->reg_port);
            if (0 == ret)
            {
                it->second->get_catalog = true;
            }
        }

        it++;
    }
}

/// 增加注册
bool CGbRegSession::add_auth_reg(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time, uint32_t reg_period)
{
    std::string device_id(msg->from->url->username);
    
    std::map<std::string, CRegNode*>::iterator it;
    it = dev_reg_queue.find(device_id);
    if (dev_reg_queue.end() == it)
    {
        if (cur_reg_dev_num < max_reg_dev_num)
        {
            return _add(device_id, msg, ip, port, time);
        }

        return false;
    }

    /// update reg msg
    osip_message_t *clone_msg   = NULL;
    if (OSIP_SUCCESS != osip_message_clone(msg, &clone_msg))
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gbsession alloc new msg failed, may be no mem\n");
    }
    else
    {
        osip_message_free(it->second->last_reg_msg);
        it->second->last_reg_msg    = clone_msg;
    }
    it->second->last_reg_time       = time;
    it->second->reg_ip              = ip;
    it->second->reg_port            = port;

    CLog::log(CLog::CLOG_LEVEL_API, "gbsession update session\n");
    return true;
}

/// 增加元素
bool CGbRegSession::_add(std::string &device_id, osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time)
{
    CRegNode *node;
    try
    {
        node = new CRegNode;
    }
    catch (...)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gbsession alloc new node failed, may be no mem\n");
        return false;
    }

    if (OSIP_SUCCESS != osip_message_clone(msg, &node->last_reg_msg))
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gbsession alloc new msg failed, may be no mem\n");
        delete node;
        return false;
    }

    node->reg_ip                = ip;
    node->reg_port              = port;
    node->last_reg_time         = time;

    dev_reg_queue[device_id]    = node;


    CLog::log(CLog::CLOG_LEVEL_API, "gbsession add new session,%s\n", device_id.c_str());
    return true;
}





/// 构造函数
CGbRegSession::CRegNode::CRegNode()
{
    last_reg_msg    = NULL;
    last_reg_time   = 0;

    alive_time      = 3600;        /// 保活时间
    authenticated   = false;        /// 是否已认证
    get_catalog     = false;

    reg_ip          = 0;
    reg_port        = 0;
}

/// 释放资源
void CGbRegSession::CRegNode::free_resource()
{
    if (last_reg_msg)
    {
        osip_message_free(last_reg_msg);
        last_reg_msg    = NULL;
    }

    alive_time      = 3600;        /// 保活时间
    authenticated   = false;        /// 是否已认证
    last_reg_time   = 0;
    get_catalog     = false;

    reg_ip          = 0;
    reg_port        = 0;
}

/// 析构函数
CGbRegSession::CRegNode::~CRegNode()
{
    free_resource();
}


