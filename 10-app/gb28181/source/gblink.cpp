#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/un.h>
#include <sys/prctl.h>

#include "oscbb.h"
#include "common.h"
#include "socklinker.h"
#include "gblink.h"

#define gbsip_log(fmt, ...) CLog::log(CLog::CLOG_LEVEL_API, "%s %u:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define sock_log(fmt, ...)  CLog::log(CLog::CLOG_LEVEL_API, "%s %d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define GBSIP_MAX_RECV_BUF_LEN     8192 


/// 国标套接口层
class CGbSockLink
{
public:
    CGbSockLink(); 

    /// init param
    int create(CGbLink::CCfgParam &param);

    /// 国标服务发送消息
    int svr_send_data(void *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /// 内部通信发送消息
    int inter_send_data(char *buf, int len, char *rmt_addr);

    /// 配置信令通道发送消息
    int external_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /// 客户端通道发送消息
    int nc_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /// 释放资源
    void free_resource();

    /// 销毁套接口链路对象
    int deinit();

    static void svr_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time);

    static void inter_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time);

    static void external_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time);

    static void nc_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time);

    static void ncproxy_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time);

private:
    /// 创建所有套接字
    int create_socket();

    /// 创建事件模型
    int create_ev_model();

    /// udp recv data & cb
    int udp_recv_data(int fd, CGbLink::FInetDataCb cb, void *cb_ctx, uint64_t cur_time);

    /// unix_datagram recv data & cb
    int udp_unix_recv_data(int fd, CGbLink::FUnixDataCb cb, void *cb_ctx, uint64_t cur_time);

    /// 套接字任务
    static void *sock_routine(void *arg);

    /// 处理epoll事件
    void sock_deal_events(int num, uint64_t time);

    /// 处理触发器
    void sock_deal_trigger(uint64_t time);
    // static void receive_data(void *context, struct epoll_event *ev);
private:
    /// epoll deal function
    typedef void (*FSockEpollDealFun)(void *ctx, struct epoll_event *ev, uint64_t cur_time);

    /// epoll data
    typedef struct tagSockEpollCtx
    {
        tagSockEpollCtx();
        int                 fd;
        FSockEpollDealFun   func;
        void                *context;
    }TSockEpollCtx;

    CGbLink::CCfgParam  cfg_param;

    /// sock link
    TSockEpollCtx       sock_svr_ctx; 
    TSockEpollCtx       sock_inter_ctx; 
    TSockEpollCtx       sock_external_ctx; 
    TSockEpollCtx       sock_ncproxy_ctx; 

    /// epoll param
    int                 ep_fd;              /// epoll fd
    uint32_t            ep_max_events;
    int                 ep_timeout;
    struct epoll_event  *ep_ev;

    struct sockaddr_in  rmt_addr;
    struct sockaddr_un  rmt_addr_unix;
    socklen_t           rmt_addr_len;

    char                buf[GBSIP_MAX_RECV_BUF_LEN];
    int                 data_len;

    uint32_t            trigger_count;

    pthread_t           ep_task_pid;
    bool                ep_task_exit;
};

/*
 * epoll task
 */
void *CGbSockLink::sock_routine(void *arg)
{
    int ret;
    uint64_t time;
    CGbSockLink *sock_link = (CGbSockLink *)arg;         // cur link
    struct epoll_event *ev = sock_link->ep_ev;

    sock_link->ep_task_exit = false;

    ret = prctl(PR_SET_NAME, "GBlink", NULL, NULL, NULL);
    if (ret < 0)
    {
        sock_log("set gb28181 stack failed,errno:%d\n", errno);
    }

    // start service
    sock_log("link:%p->%d running\n", sock_link, sock_link->ep_fd);
    while (true)
    {
        ret = epoll_wait(sock_link->ep_fd, ev, sock_link->ep_max_events, sock_link->ep_timeout);
        time = CBB::gettime_ms();
        if (ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }

            sock_log("link:%p->%d epoll wait failed, task to exit, errno:%d\n", sock_link, sock_link->ep_fd, errno);
            break;
        }
        else if (ret > 0)
        {
            // deal event
            sock_link->sock_deal_events(ret, time);
        }

        // 处理触发器
        sock_link->sock_deal_trigger(time);
    }

    sock_log("link:%p->%d task exit now\n", sock_link, sock_link->ep_fd);
    sock_link->ep_task_exit = true;

    return NULL;
}

/*
 * deal epoll trigger 
 */
void CGbSockLink::sock_deal_trigger(uint64_t time)
{
    trigger_count++;

    if (cfg_param.event_trigger_cb)
    {
        cfg_param.event_trigger_cb(cfg_param.ev_trigger_ctx, time);
    }
}

/*
 * deal epoll events
 */
void CGbSockLink::sock_deal_events(int num, uint64_t time)
{
    int i;
    struct epoll_event ev;
    TSockEpollCtx *ctx;

    // deal come data
    for (i = 0; i < num; i++)
    {
        ev = *(ep_ev + i);           // copy avoid mangle
        if (NULL != ev.data.ptr)
        {
            ctx = (TSockEpollCtx *)ev.data.ptr;
            /// 这里没有错误处理情况,需要注意,用户处理函数需要处理错误事件
            ctx->func(ctx->context, &ev, time);
        }
        else
        {
           sock_log("fatal error happened, epoll event with no user\n");
        }
    }
}

/// server recv data
int CGbSockLink::udp_recv_data(int fd, CGbLink::FInetDataCb cb, void *cb_ctx, uint64_t cur_time)
{
    rmt_addr_len    = sizeof(rmt_addr);
    data_len = CBB::cbb_recvfrom(fd, buf, sizeof(buf), &rmt_addr, (int *)&rmt_addr_len, MSG_TRUNC);
    if (data_len > 0)
    {
        if ((unsigned)data_len <= sizeof(buf))
        {
            cb(cb_ctx, buf, data_len, rmt_addr.sin_addr.s_addr, rmt_addr.sin_port, cur_time);
        }
        else
        {
            /// too larger data
            CLog::log(CLog::CLOG_LEVEL_API, "server fd recv too larger data,len:%d\n", data_len);
        }
    }
    else if (data_len < 0)
    {
        /// need to restart linker
        return -1;
    }

    return 0;
}

/// ncproxy recv data
void CGbSockLink::ncproxy_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time)
{
    CGbSockLink     *gb_link    = static_cast<CGbSockLink *>(ctx); 

    int ret = gb_link->udp_recv_data(gb_link->sock_ncproxy_ctx.fd, gb_link->cfg_param.ncproxy_data_cb,
                                        gb_link->cfg_param.ncproxy_data_cb_ctx, cur_time);
    if (ret < 0)
    {
        /// need to restart cur link
        CLog::log(CLog::CLOG_LEVEL_API, "ncproxy fd recv data fialed:%d\n", gb_link->sock_ncproxy_ctx.fd);
    }
}

/// server recv data
void CGbSockLink::svr_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time)
{
    CGbSockLink     *gb_link    = static_cast<CGbSockLink *>(ctx); 

    int ret = gb_link->udp_recv_data(gb_link->sock_svr_ctx.fd, gb_link->cfg_param.gb_svr_data_cb, gb_link->cfg_param.gb_svr_data_cb_ctx, cur_time);
    if (ret < 0)
    {
        /// need to restart cur link
        CLog::log(CLog::CLOG_LEVEL_API, "server fd recv data fialed:%d\n", gb_link->sock_svr_ctx.fd);
    }
}

/// server recv data
int CGbSockLink::udp_unix_recv_data(int fd, CGbLink::FUnixDataCb cb, void *cb_ctx, uint64_t cur_time)
{
    data_len = CBB::cbb_recvfrom(fd, buf, sizeof(buf), &rmt_addr_unix, (int *)&rmt_addr_len, MSG_TRUNC);
    if (data_len > 0)
    {
        if ((unsigned)data_len <= sizeof(buf))
        {
            cb(cb_ctx, buf, data_len, rmt_addr_unix.sun_path, cur_time);
        }
        else
        {
            /// too larger data
            CLog::log(CLog::CLOG_LEVEL_API, "server unix fd recv too larger data,len:%d\n", data_len);
        }
    }
    else if (data_len < 0)
    {
        /// need to restart linker
        return -1;
    }

    return 0;
}

/// server recv data
void CGbSockLink::inter_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time)
{
    CGbSockLink     *gb_link    = static_cast<CGbSockLink *>(ctx); 

    int ret = gb_link->udp_unix_recv_data(gb_link->sock_inter_ctx.fd, gb_link->cfg_param.inter_msg_cb, gb_link->cfg_param.inter_msg_cb_ctx, cur_time);
    if (ret < 0)
    {
        /// need to restart cur link
    }
}

/// init param
int CGbSockLink::create(CGbLink::CCfgParam &param)
{
    int ret;

    ret = create_ev_model();
    if (ret < 0)
    {
        return ret;
    }

    cfg_param   = param;
    ret = create_socket();
    if (ret < 0)
    {
        return ret;
    }

    ret = pthread_create(&ep_task_pid, NULL, CGbSockLink::sock_routine, this);

    return ret;
}

/// 创建事件模型
int CGbSockLink::create_ev_model()
{
    ep_fd = epoll_create1(EPOLL_CLOEXEC);
    if (ep_fd < 0)
    {
        sock_log("create epoll failed,flag:%d,errno:%d\n", 0, errno);
        return -1;
    }

    ep_ev = (struct epoll_event *)malloc(sizeof(struct epoll_event) * ep_max_events);
    if (NULL == ep_ev)
    {
        sock_log("malloc failed, no mem, events num:%u\n", ep_max_events);
        return -ENOMEM;
    }

    return 0;
}

/// 创建套接字
int CGbSockLink::create_socket()
{
    int ret;
    struct sockaddr_in addr;

    struct epoll_event  evt; 
    
    if (0 != cfg_param.svr_port)
    {
        addr.sin_family         = AF_INET;
        addr.sin_addr.s_addr    = cfg_param.svr_addr;
        addr.sin_port           = cfg_param.svr_port;
        sock_svr_ctx.fd         = CBB::cbb_sock_create(AF_INET, SOCK_DGRAM, 0, 1, &addr, sizeof(addr));
        if (sock_svr_ctx.fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "gblink create socket failed\n");
            return -1;
        }

        sock_svr_ctx.func       = svr_recv_data;
        sock_svr_ctx.context    = this;
        evt.events              = EPOLLIN;
        evt.data.ptr            = &sock_svr_ctx;
        ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_svr_ctx.fd, &evt);
        if (ret < 0)
        {
            sock_log("sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                        ep_fd, sock_svr_ctx.fd, errno);
            return -1;
        }
        CLog::log(CLog::CLOG_LEVEL_API, "gblink create svr socket:%d\n", sock_svr_ctx.fd);
    }

    if (0 != cfg_param.inter_msg_addr[0])
    {
        struct sockaddr_un  addr_un;
        addr_un.sun_family         = AF_UNIX;
        snprintf(addr_un.sun_path, sizeof(addr_un.sun_path), "%s", cfg_param.inter_msg_addr);
        unlink(cfg_param.inter_msg_addr);
        sock_inter_ctx.fd = CBB::cbb_sock_create(AF_UNIX, SOCK_DGRAM, 0, 1, &addr_un, sizeof(addr_un));
        if (sock_inter_ctx.fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "gblink create socket failed\n");
            return -1;
        }

        sock_inter_ctx.func     = inter_recv_data;
        sock_inter_ctx.context  = this;
        evt.events              = EPOLLIN;
        evt.data.ptr            = &sock_inter_ctx;
        ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_inter_ctx.fd, &evt);
        if (ret < 0)
        {
            sock_log("sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                        ep_fd, sock_svr_ctx.fd, errno);
            return -1;
        }
    }

    if (0 != cfg_param.external_port)
    {
        addr.sin_family         = AF_INET;
        addr.sin_addr.s_addr    = cfg_param.external_addr;
        addr.sin_port           = cfg_param.external_port;
        sock_external_ctx.fd = CBB::cbb_sock_create(AF_INET, SOCK_STREAM, 0, 1, &addr, sizeof(addr));
        if (sock_external_ctx.fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "gblink create socket failed\n");
            return -1;
        }
    }

    if (0 != cfg_param.ncproxy_port)
    {
        addr.sin_family         = AF_INET;
        addr.sin_addr.s_addr    = cfg_param.ncproxy_addr;
        addr.sin_port           = cfg_param.ncproxy_port;
        sock_ncproxy_ctx.fd = CBB::cbb_sock_create(AF_INET, SOCK_DGRAM, 0, 1, &addr, sizeof(addr));
        if (sock_ncproxy_ctx.fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "gblink create socket failed\n");
            return -1;
        }

        sock_ncproxy_ctx.func       = ncproxy_recv_data;
        sock_ncproxy_ctx.context    = this;
        evt.events              = EPOLLIN;
        evt.data.ptr            = &sock_ncproxy_ctx;
        ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_ncproxy_ctx.fd, &evt);
        if (ret < 0)
        {
            sock_log("sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                        ep_fd, sock_ncproxy_ctx.fd, errno);
            return -1;
        }
        CLog::log(CLog::CLOG_LEVEL_API, "gblink create ncproxy socket:%d\n", sock_ncproxy_ctx.fd);
    }

    return 0;
}

/// 构造函数
CGbSockLink::CGbSockLink()
{
    memset(&cfg_param, 0, sizeof(cfg_param));

    ep_fd           = -1;
    ep_max_events   = 5;            /// current cannot be set
    ep_timeout      = 100;          /// 100ms
    ep_ev           = NULL;

    rmt_addr_len    = 0;
    memset(&rmt_addr, 0, sizeof(rmt_addr));
    memset(&rmt_addr_unix, 0, sizeof(rmt_addr_unix));

    trigger_count   = 0;;
}

/// 国标服务发送消息
int CGbSockLink::svr_send_data(void *buf, int len, uint32_t rmt_ip, uint16_t rmt_port)
{
    struct sockaddr_in  addr;
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = rmt_ip;
    addr.sin_port           = rmt_port;
    uint32_t addr_len       = sizeof(addr);

    return CBB::cbb_sendto(sock_svr_ctx.fd, buf, len, &addr, addr_len);
}

/// 内部通信发送消息
int CGbSockLink::inter_send_data(char *buf, int len, char *rmt_addr)
{
    struct sockaddr_un  addr;
    addr.sun_family         = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", rmt_addr);
    uint32_t addr_len       = sizeof(addr);

    return CBB::cbb_sendto(sock_inter_ctx.fd, buf, len, &addr, addr_len);
}

/// 配置信令通道发送消息
int CGbSockLink::external_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port)
{
    return 0;
}

/// 客户端通道发送消息
int CGbSockLink::nc_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port)
{
    return 0;
}


CGbSockLink::tagSockEpollCtx::tagSockEpollCtx()
{
    fd          = -1;
    func        = NULL;
    context     = NULL;
}

/// 构造函数
CGbLink::CGbLink()
{
    sock_link = NULL;
}

int CGbLink::create(CCfgParam &param)
{
    try
    {
        sock_link   = new CGbSockLink;
    }
    catch (...)
    {
        return -ENOMEM;
    }

    int ret = sock_link->create(param);
    if (0 != ret)
    {
        delete sock_link;
    }

    return ret;
}

/// 国标服务发送消息
int CGbLink::svr_send_data(void *buf, int len, uint32_t rmt_ip, uint16_t rmt_port)
{
    return sock_link->svr_send_data(buf, len, rmt_ip, rmt_port);
}

/// 内部通信发送消息
int CGbLink::inter_send_data(char *buf, int len, char *rmt_addr)
{
    return sock_link->inter_send_data(buf, len, rmt_addr);
}

CGbLink::CCfgParam::CCfgParam()
{ 
    svr_addr                = 0;            /// 国标服务地址
    svr_port                = 0;            /// 国标服务端口
    gb_svr_data_cb          = NULL;         /// 国标回调
    gb_svr_data_cb_ctx      = NULL;    /// 国标回调用户上下文

    memset(inter_msg_addr, 0, sizeof(inter_msg_addr));     /// 内部通信端口,进程内部使用UNIX域UDP通信方式
    inter_msg_cb            = NULL;           /// 内部消息回调
    inter_msg_cb_ctx        = NULL;      /// 内部消息回调用户上下文

    external_addr           = 0;          /// 国标服务器配置信令通信地址
    external_port           = 0;          /// 国标服务器配置信令通信端口
    external_data_cb        = NULL;       /// 国标回调
    external_data_cb_ctx    = NULL;  /// 国标回调用户上下文

    ncproxy_addr            = 0;           /// NC客户端地址
    ncproxy_port            = 0;           /// NC客户端端口
    ncproxy_data_cb         = NULL;        /// 国标回调
    ncproxy_data_cb_ctx     = NULL;   /// 国标回调用户上下文

    event_trigger_cb        = NULL;       /// 触发器
    ev_trigger_ctx          = NULL;        /// 触发器用户上下文
}
