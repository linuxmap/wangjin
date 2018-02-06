

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/epoll.h>

#include "msgno.h"
#include "vtdu.h"
#include "dataswitch.h"

#define vtdu_log(fmt, ...)  printf("[VTDU]" fmt, ##__VA_ARGS__)

/// VTDU管理
class CVtduMgr
{
public:
    CVtduMgr();

    /// 初始化VTDU模块
    int init(const VTDU::CVtduCfgParam &param);


private:
    /// epoll info
    struct CEpollData
    {
        int                 ep_fd;
        struct epoll_event  *ep_ev;
        int                 ep_max_events;
        int                 ep_timeout;
    }ep_data;
    pthread_t               task_id;
    uint8_t                 recv_buf[1500];
    ssize_t                 recv_len;

    /// 消息中心通信协议分析器
    MN::CMsgParser          mc_msg_parser;
    
    /// dataswitch info
    DSID                ds_id;
    TDSNetAddr          *recv_grp;

    VTDU::CVtduCfgParam cfg_param;

private:
    /// vtdu 管理线程
    static void *mgr_routine(void *arg);

    /// 消息处理
    static void msg_handler(void *ctx, MN::CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time);

    /// 消息处理
    void deal_one_msg(MN::CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time);

    /// vtdu 管理线程
    void routine();

    /// 增加转发
    void deal_add_switch_ipv4(const MN::CMsgAddSwitchIpv4 &msg);

    /// delete switch channel
    void deal_del_switch_ipv4(const MN::CMsgAddSwitchIpv4 &msg);
}vtdu_mgr;

/// 构造函数
CVtduMgr::CVtduMgr()
{
    ep_data.ep_fd           = -1;
    ep_data.ep_ev           = NULL;
    ep_data.ep_max_events   = 4;
    ep_data.ep_timeout      = 100;

    ds_id                   = INVALID_DSID;
    recv_grp                = NULL;
}

/// 消息处理
void CVtduMgr::msg_handler(void *ctx, MN::CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time)
{
    CVtduMgr *it    = static_cast<CVtduMgr *>(ctx);
    it->deal_one_msg(hdr, msg_buf, msg_len, cur_time);
}

/// 消息处理
void CVtduMgr::deal_one_msg(MN::CMsgHdr &msg_hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time)
{
    if (MC_MSG_PROTOCOL_VER01 != msg_hdr.ver)
    {
        vtdu_log("invalid msg ver:%d\n", msg_hdr.ver);
        return;
    }

    bool ret;
    vtdu_log("deal msg ver:%u\n", msg_hdr.type);
    switch (msg_hdr.type)
    {
    case MN_VTDU_ADD_SWITCH_IPV4:
        {
            MN::CMsgAddSwitchIpv4   msg_addswitch;
            ret    = msg_addswitch.buf2msg(msg_buf, msg_len, msg_addswitch);
            if (true == ret)
            {
                deal_add_switch_ipv4(msg_addswitch);
            }
            else
            {
                vtdu_log("MN_VTDU_ADD_SWITCH_IPV4:msg to buffer failed,len:%u\n", msg_len);
            }
        }
        break;

    case MN_VTDU_DEL_SWITCH_IPV4:
        {
            MN::CMsgAddSwitchIpv4   msg_addswitch;
            ret    = msg_addswitch.buf2msg(msg_buf, msg_len, msg_addswitch);
            if (true == ret)
            {
                deal_del_switch_ipv4(msg_addswitch);
            }
            else
            {
                vtdu_log("MN_VTDU_ADD_SWITCH_IPV4:msg to buffer failed,len:%u\n", msg_len);
            }
        }
        break;

    default:
        vtdu_log("undeal msg type:%u\n", msg_hdr.type);
        break;
    }

    if (false == ret)
    {
        vtdu_log("msg type:%d data maybe cracked\n", msg_hdr.type);
    }
}


/// delete switch channel
void CVtduMgr::deal_del_switch_ipv4(const MN::CMsgAddSwitchIpv4 &msg)
{
    TDSNetAddr tSendToAddr;

    if (msg.channel_id < cfg_param.channel_num)
    {
        tSendToAddr.dwIP    = msg.to_ip;
        tSendToAddr.wPort   = msg.to_port;
        vtdu_log("ds remove many to one port:%u\n", tSendToAddr.wPort);
        uint32_t ret = dsRemoveManyToOne(ds_id, recv_grp[msg.channel_id], tSendToAddr);
        if (DSOK != ret)
        {
            vtdu_log("ds remove many to one failed\n");
        }
    }
    else
    {
        vtdu_log("remove switch ipv4 channel invalid:%d\n", msg.channel_id);
    }
}

/// 增加转发
void CVtduMgr::deal_add_switch_ipv4(const MN::CMsgAddSwitchIpv4 &msg)
{
    TDSNetAddr tSendToAddr;

    if (msg.channel_id < cfg_param.channel_num)
    {
        tSendToAddr.dwIP    = msg.to_ip;
        tSendToAddr.wPort   = msg.to_port;
        vtdu_log("ds add many to one port:%u\n", tSendToAddr.wPort);
        uint32_t ret = dsAddManyToOne(ds_id, recv_grp[msg.channel_id], 0, tSendToAddr, 0);
        if (DSOK != ret)
        {
            vtdu_log("ds add many to one failed\n");
        }
    }
    else
    {
        vtdu_log("add switch ipv4 channel invalid:%d\n", msg.channel_id);
    }
}

/// vtdu 管理线程
void CVtduMgr::routine()
{
    int ret;
    uint64_t cur_time;
    struct timespec ts;

    ret = epoll_wait(ep_data.ep_fd, ep_data.ep_ev, ep_data.ep_max_events, ep_data.ep_timeout);
    clock_gettime(CLOCK_MONOTONIC, &ts);
    cur_time = (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec >> 20);
    if (ret < 0)
    {
        if (EINTR == errno)
        {
            return;
        }

        vtdu_log("mgr epoll wait failed, task to exit, errno:%d\n", errno);
        return;
    }

    for (int i = 0; i < ret; i++)
    {
        if ( (ep_data.ep_ev[i].events & EPOLLERR) || (ep_data.ep_ev[i].events & EPOLLHUP) )
        {
            vtdu_log("epoll wait,event %u\n", ep_data.ep_ev[i].events);

            vtdu_log("fatal error happened\n");
        }
        else if (ep_data.ep_ev[i].events & EPOLLIN)
        {
            recv_len = recv(ep_data.ep_ev[i].data.fd, recv_buf, sizeof(recv_buf), 0);
            vtdu_log("vtdu recv data,len:%ld\n", recv_len);
            if (recv_len > 0)
            {
                /// 处理内部协议
                mc_msg_parser.parser_income_data(recv_buf, uint32_t(recv_len), cur_time);
                /// parser_inter_data(cur_time);
            }
            else
            {
                vtdu_log("recv ret:%ld,errno:%d\n", recv_len, errno);
            }
        }
    }
}

/// vtdu 管理线程
void *CVtduMgr::mgr_routine(void *arg)
{
    vtdu_log("vtdu manager task run\n");
    int ret = prctl(PR_SET_NAME, "VtduMgrTask", NULL, NULL, NULL);
    if (ret < 0)
    {
        vtdu_log("set dev mgr task name failed,errno:%d\n", errno);
    }

    CVtduMgr *p = (CVtduMgr *)arg;

    while(true)
    {
        p->routine();
    }

    vtdu_log("msg server exit now\n");

    return NULL;
}

/// 初始化VTDU模块
int CVtduMgr::init(const VTDU::CVtduCfgParam &param)
{
    int ret;

    if (param.inter_fd < 0)
    {
        return -1;
    }

    /// 创建协议处理器
    if (mc_msg_parser.create() < 0)
    {
        vtdu_log("stream msg parser create failed\n");
        return -1;
    }

    /// 消息处理句柄
    mc_msg_parser.set_msg_handler(msg_handler, this);

    /// set
    cfg_param   = param;
    cfg_param.channel_num = (cfg_param.channel_num > 1024) ? 1024 : cfg_param.channel_num;
    cfg_param.channel_num = (cfg_param.channel_num < 1) ? 1 : cfg_param.channel_num;
    recv_grp    = (TDSNetAddr *)malloc(sizeof(*recv_grp) * cfg_param.channel_num);
    if (NULL == recv_grp)
    {
        vtdu_log("malloc recvgrp failed:%d\n", cfg_param.channel_num);
        return -ENOMEM;;
    }

    ds_id = dsCreate(0, NULL);
    if (INVALID_DSID == ds_id)
    {
        return -1;
    }

    for (uint32_t i = 0; i < cfg_param.channel_num; i++)
    {
        recv_grp[i].dwIP = INADDR_ANY;
        recv_grp[i].wPort = 10000 + i * 10; 

        if (DSOK != dsAddDump(ds_id, recv_grp[i], 0))
        {
            printf("ds add failed\n");
            return -1;
        }
    }

    ep_data.ep_fd   = epoll_create1(EPOLL_CLOEXEC);
    if (ep_data.ep_fd < 0)
    {
        vtdu_log("create epoll failed,errno:%d\n", errno);
        return -1;
    }
    
    ep_data.ep_ev   = (struct epoll_event *)malloc(sizeof(struct epoll_event) * ep_data.ep_max_events);
    if (NULL == ep_data.ep_ev)
    {
        vtdu_log("malloc epoll events failed,errno:%d\n", errno);
        return -ENOMEM;
    }

    struct epoll_event ev_tmp;
    memset(&ev_tmp, 0, sizeof(ev_tmp));
    ev_tmp.events   = EPOLLIN;
    ev_tmp.data.fd  = cfg_param.inter_fd;
    ret = epoll_ctl(ep_data.ep_fd, EPOLL_CTL_ADD, cfg_param.inter_fd, &ev_tmp);
    if (ret < 0)
    {
        vtdu_log("epoll_ctl failed,errno:%d\n", errno);
    }

    ret = pthread_create(&task_id, NULL, CVtduMgr::mgr_routine, this);    
    if (0 != ret)
    {
        vtdu_log("create mgr task failed,errno:%d\n", ret);
        return -ret;
    }

    return 0;
}


namespace VTDU
{

/// 媒体分发模块初始化
int vtdu_init(const CVtduCfgParam &param)
{
    return vtdu_mgr.init(param);
}

CVtduCfgParam::CVtduCfgParam()
{
    inter_fd    = -1;
}

}
