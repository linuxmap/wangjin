#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "oscbb.h"
#include "common.h"
#include "msgno.h"
#include "mc.h"

static class CMsgServer
{
public:
    CMsgServer();

    int create(const MC::CMcCfgParam &param);

    /// run func
    void *run();

    /// run routine
    static void *routine(void *arg);

    /// start to recv msg
    int start();

    /// stop to recv msg
    int stop();

    /// destroy current obj
    int destroy();

    /// set msg cb
    int set_msg_cb(void *ctx, void (*cb)(void *ctx, MN::CMsgFormat &msg, uint64_t time));

    /// send data to gb module
    int send_to_gb(void *buf, int len, void *ex_buf, int ex_len);

    /// 发送数据给vtdu模块
    int send_to_vtdu(uint32_t msg_type, void *buf, int len, void *ex_buf, int ex_len);
private:

    /// unix sock recv data
    static void unix_recvfrom_data(void *ctx, struct epoll_event *ev, uint64_t cur_time);

    /// unix sock recv data
    static void vtdu_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time);
    
    /// ipv4 socket recv data
    static void unix_new_connection(void *ctx, struct epoll_event *ev, uint64_t cur_time);

    /// 处理套接字事件
    void sock_deal_events(int num, uint64_t time);

    /// 消息处理器
    static void vtdu_msg_handler(void *ctx, MN::CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time);

    /// 消息处理
    void deal_one_msg(MN::CMsgHdr &msg_hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time);

    /// epoll deal function
    typedef void (*FSockEpollDealFun)(void *ctx, struct epoll_event *ev, uint64_t cur_time);


    /// epoll data
    struct CSockEpollCtx
    {
        CSockEpollCtx()
        {
            fd      = -1;
            func    = NULL;
            context = NULL;
        }
        int                 fd;
        FSockEpollDealFun   func;
        void                *context;
    };

    /// for client
    int client_fd[10];

    pthread_t       pid;
    pthread_mutex_t mutex;

    MC::CMcCfgParam     cfg_param;

#define UNIX_DGRAM_CHANNEL      0
#define UNIX_STREAM_CHANNEL     1
#define IPV4_STREAM_CHANNEL     2
#define UNIX_VTDU_CHANNEL       3
    CSockEpollCtx       sock_ctx[4];

    int                 ep_fd;
    struct epoll_event  *ep_ev; 
    int                 ep_max_events;
    int                 ep_timeout;

    uint8_t             buf[8192];
    ssize_t             recv_len;

    uint8_t             vtdu_snd_buf[2048];
    ssize_t             vtdu_snd_data_len;
    uint16_t            vtdu_send_seq;

    char                snd_buf[8192];
    int                 snd_len;

    bool                task_run;
    bool                task_exit;

    /// 消息中心通信协议分析器
    MN::CMsgParser          vtdu_msg_parser;

    MN::CMsgFormat          recv_msg;

    void                (*msg_cb)(void *ctx, MN::CMsgFormat &msg, uint64_t time);
    void                *msg_cb_ctx;
private:
    void unix_dgram_deal_msg();

    void call_back_msg(MN::CMsgFormat &msg);

    /// 创建套接字
    int create_sock();

    /// 关闭套接字
    void close_sock();

}msg_server;

/// set msg cb
int CMsgServer::set_msg_cb(void *ctx, void (*cb)(void *ctx, MN::CMsgFormat &msg, uint64_t time))
{
    msg_cb      = cb;
    msg_cb_ctx  = ctx;
    return 0;
}


/// msg callback
void CMsgServer::call_back_msg(MN::CMsgFormat &msg)
{
    if (NULL != msg_cb)
    {
        msg_cb(msg_cb_ctx, msg, 0);
    }
}

CMsgServer::CMsgServer()
{
    ep_max_events       = 4;
    ep_timeout          = 100;
    task_exit           = false;
    task_run            = false;
    vtdu_snd_data_len   = 0;
    vtdu_send_seq       = 0;
}

/// start to recv msg
int CMsgServer::start()
{
    CGuard::enterCS(mutex);
    task_run    = true;
    CGuard::leaveCS(mutex);

    return 0;
}

/// stop to recv msg
int CMsgServer::stop()
{
    CGuard::enterCS(mutex);
    task_run    = false;
    CGuard::leaveCS(mutex);

    return 0;
}


void CMsgServer::unix_dgram_deal_msg()
{
    CLog::log(CLog::CLOG_LEVEL_API, "mc to deal msg\n");

    // check msg len 
    if (recv_len < (int)sizeof(recv_msg))
    {
        CLog::log(CLog::CLOG_LEVEL_API, "mc,invalid msg len:%d\n", recv_len);
        return;
    }

    // check data len
    memcpy(&recv_msg, buf, sizeof(recv_msg));
    if (recv_msg.var_len + (int)sizeof(recv_msg) != recv_len)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "mc,invalid msg len:%d, var len:%d\n", recv_len, recv_msg.var_len);
        return;
    }

    recv_msg.var    = buf + sizeof(recv_msg);

    call_back_msg(recv_msg);

}

/// destroy current obj
int CMsgServer::destroy()
{
    CGuard::enterCS(mutex);
    task_exit    = true;
    CGuard::leaveCS(mutex);

#if 0
    // to check whether exit
    while (true)
    {
        CGuard::enterCS(mutex);
        if (-1 == unix_dgram_fd)
        {
            CGuard::leaveCS(mutex);
            break;
        }

        CGuard::leaveCS(mutex);

        CLog::log(CLog::CLOG_LEVEL_API, "to wait msg server exit\n");
        usleep(10000);
    }
#endif

    pthread_mutex_destroy(&mutex);

    return 0;
}

/*
 * deal epoll events
 */
void CMsgServer::sock_deal_events(int num, uint64_t time)
{
    int i;
    struct epoll_event ev;
    CSockEpollCtx *ctx;

    // deal come data
    for (i = 0; i < num; i++)
    {
        ev = *(ep_ev + i);           // copy avoid mangle
        if (NULL != ev.data.ptr)
        {
            ctx = (CSockEpollCtx *)ev.data.ptr;
            /// 这里没有错误处理情况,需要注意,用户处理函数需要处理错误事件
            ctx->func(ctx->context, &ev, time);
        }
        else
        {
           CLog::log(CLog::CLOG_LEVEL_API, "fatal error happened, epoll event with no user\n");
        }
    }
}

void *CMsgServer::run()
{
    uint64_t cur_time;
    CLog::log(CLog::CLOG_LEVEL_API, "msg server run\n");
    int ret = prctl(PR_SET_NAME, "MsgServerTask", NULL, NULL, NULL);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "set dev mgr task name failed,errno:%d\n", errno);
    }

    while(true)
    {
        CGuard::enterCS(mutex);

        if (true == task_exit)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "msgserver is commanded to exit\n");
            CGuard::leaveCS(mutex);
            break;
        }

        // if not start,don't recv msg
        if (false == task_run)
        {
            CGuard::leaveCS(mutex);
            usleep(10000);
            continue;
        }

        ret = epoll_wait(ep_fd, ep_ev, ep_max_events, ep_timeout);
        cur_time = CBB::gettime_ms();
        if (ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }

            CLog::log(CLog::CLOG_LEVEL_API, "MsgServer epoll wait failed, task to exit, errno:%d\n", errno);
            break;
        }
        else if (ret > 0)
        {
            // deal event
            sock_deal_events(ret, cur_time);
        }

        /// shedule must be called here

        CGuard::leaveCS(mutex);
    }

    CLog::log(CLog::CLOG_LEVEL_API, "msg server exit now\n");
    return NULL;
}

void *CMsgServer::routine(void *arg)
{
    CMsgServer *param = static_cast<CMsgServer *>(arg);
    return param->run();
}

/// 发送数据给vtdu模块
int CMsgServer::send_to_vtdu(uint32_t msg_type, void *buf, int len, void *ex_buf, int ex_len)
{
    int ret = -1;
    switch (msg_type)
    {
    case MN_VTDU_ADD_SWITCH_IPV4:
    case MN_VTDU_DEL_SWITCH_IPV4:
        {
            MN::CMsgAddSwitchIpv4 *msg2vtdu = (MN::CMsgAddSwitchIpv4 *)buf;
            if (0 != vtdu_snd_data_len)
            {
                CLog::log(CLog::CLOG_LEVEL_API, "[MC]vtdu send buf not empty,:%u\n", vtdu_snd_data_len);
                break;
            }

            /// 转换头部到缓冲中
            MN::CMsgHdr msg_hdr(vtdu_send_seq++, msg_type);
            vtdu_snd_data_len   = 4;
            uint32_t write_len = sizeof(vtdu_snd_buf) - 4;
            if (false == msg_hdr.msg2buf_calc_crc(vtdu_snd_buf + vtdu_snd_data_len, write_len, msg_hdr))
            {
                break;
            }

            /// 转换消息到缓冲中
            vtdu_snd_data_len += write_len; 
            write_len = sizeof(vtdu_snd_buf) - vtdu_snd_data_len; 
            if (false == msg2vtdu->msg2buf(vtdu_snd_buf + vtdu_snd_data_len, write_len, *msg2vtdu))
            {
                vtdu_snd_data_len = 0;
                break;
            }
            
            /// send
            vtdu_snd_data_len   += write_len;

            /// add header
            MN::CStreamHdr str_hdr;
            str_hdr.len         = vtdu_snd_data_len - 4; 
            str_hdr.hdr2buf(vtdu_snd_buf, str_hdr);

            /// need to deal with non-zero lenbuf
            ret = CBB::cbb_send(sock_ctx[UNIX_VTDU_CHANNEL].fd, vtdu_snd_buf, vtdu_snd_data_len);
            if (ret < 0)
            {
                CLog::log(CLog::CLOG_LEVEL_API, "[MC]send data to vtdu failed,ret:%d\n", ret);
            }
            vtdu_snd_data_len   = 0;
            CLog::log(CLog::CLOG_LEVEL_API, "[MC]send data to vtdu successfully,ret:%d,len:%d\n", ret, vtdu_snd_data_len);
        }
        break;


    default:
        ret = -1;
        break;
    }

    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "[MC]send data to vtdu failed, msg type:%u\n", msg_type);
    }

    return ret;
}

/// send data to gb module
int CMsgServer::send_to_gb(void *buf, int len, void *ex_buf, int ex_len)
{
    if (len + ex_len > (int)sizeof(snd_buf))
    {
        return -E2BIG;
    }

    memcpy(snd_buf, buf, len);

    if ( (ex_buf) && (0 != ex_len) )
    {
        memcpy(snd_buf + len, ex_buf, ex_len);
    }

    struct sockaddr_un addr;
    addr.sun_family    = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", "/var/run/adp_gb_0");

    return CBB::cbb_sendto(sock_ctx[UNIX_DGRAM_CHANNEL].fd, snd_buf, len + ex_len, (struct sockaddr *)&addr, (socklen_t)sizeof(addr));
}

/// 消息处理器
void CMsgServer::vtdu_msg_handler(void *ctx, MN::CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time)
{
    CMsgServer *it    = static_cast<CMsgServer *>(ctx);
    it->deal_one_msg(hdr, msg_buf, msg_len, cur_time);
}

/// 消息处理
void CMsgServer::deal_one_msg(MN::CMsgHdr &msg_hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time)
{
    if (MC_MSG_PROTOCOL_VER01 != msg_hdr.ver)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "invalid msg ver:%d\n", msg_hdr.ver);
        return;
    }

    bool ret = true;
    CLog::log(CLog::CLOG_LEVEL_API, "deal msg ver:%u\n", msg_hdr.type);
    switch (msg_hdr.type)
    {
    case MN_VTDU_ADD_SWITCH_IPV4:
        {
        }
        break;

    case MN_VTDU_DEL_SWITCH_IPV4:
        break;

    default:
        CLog::log(CLog::CLOG_LEVEL_API, "undeal msg type:%u\n", msg_hdr.type);
        break;
    }

    if (false == ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "msg type:%d data maybe cracked\n", msg_hdr.type);
    }
}

int CMsgServer::create(const MC::CMcCfgParam &param)
{
    int ret;

    cfg_param   = param;

    ret = pthread_mutex_init(&mutex, NULL);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "mutex init failed,ret:%d\n", ret);
        return -ret;
    }

    /// 创建协议处理器
    if (vtdu_msg_parser.create() < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "stream msg parser create failed\n");
        return -1;
    }

    /// 消息处理句柄
    vtdu_msg_parser.set_msg_handler(vtdu_msg_handler, this);

    ret = create_sock();
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "create socket failed,ret:%d\n", ret);
        pthread_mutex_destroy(&mutex);
    }

    ret = pthread_create(&pid, NULL, CMsgServer::routine, this);
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "create thread failed,ret:%d\n", ret);
        pthread_mutex_destroy(&mutex);
    }

    return -ret;
}

/// unix sock recv data
void CMsgServer::vtdu_recv_data(void *ctx, struct epoll_event *ev, uint64_t cur_time)
{
    CMsgServer  *msg_server = static_cast<CMsgServer *>(ctx); 

    msg_server->recv_len = recv(msg_server->cfg_param.channel_vtdu_fd, msg_server->buf, sizeof(msg_server->buf), 0);
    if (msg_server->recv_len < 0)
    {
        if ( (EINTR == errno) || (EAGAIN == errno) )
        {
            return;
        }

        CLog::log(CLog::CLOG_LEVEL_API, "msgcenter recvfrom failed, task to exit, errno:%d\n", errno);
    }
    else if (0 == msg_server->recv_len)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "msgcenter vtdu channel recv ret:0,peer maybe closed, errno:%d\n", errno);
    }
    else
    {
        
        msg_server->vtdu_msg_parser.parser_income_data(msg_server->buf, uint32_t(msg_server->recv_len), cur_time);
    }
}

/// msg server recv data
void CMsgServer::unix_recvfrom_data(void *ctx, struct epoll_event *ev, uint64_t cur_time)
{
    CMsgServer  *msg_server = static_cast<CMsgServer *>(ctx); 

    struct sockaddr_un  peer_addr;
    socklen_t           addr_len;
    addr_len    = sizeof(peer_addr);
    msg_server->recv_len = recvfrom(msg_server->sock_ctx[UNIX_DGRAM_CHANNEL].fd, msg_server->buf, sizeof(msg_server->buf),
                                        0, (struct sockaddr *)&peer_addr, &addr_len);
    if (msg_server->recv_len < 0)
    {
        if ( (EINTR == errno) || (EAGAIN == errno) )
        {
            return;
        }

        CLog::log(CLog::CLOG_LEVEL_API, "msgcenter recvfrom failed, task to exit, errno:%d\n", errno);
    }
    else if (0 == msg_server->recv_len)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "msgcenter recvfrom 0, errno:%d\n", errno);
    }
    else
    {
        msg_server->unix_dgram_deal_msg();
    }
}

/// msg server recv data
void CMsgServer::unix_new_connection(void *ctx, struct epoll_event *ev, uint64_t cur_time)
{
    /// include ipv4 & unix
    /// CMsgServer  *msg_server = static_cast<CMsgServer *>(ctx); 

}

/// 创建套接字并加入到epoll列表
int CMsgServer::create_sock()
{
    int ret;
    struct epoll_event  evt;

    ep_fd = epoll_create1(EPOLL_CLOEXEC);
    if (ep_fd < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "create epoll failed,flag:%d,errno:%d\n", 0, errno);
        return -1;
    }

    ep_ev = (struct epoll_event *)malloc(sizeof(struct epoll_event) * ep_max_events);
    if (NULL == ep_ev)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "malloc failed, no mem, events num:%u\n", ep_max_events);
        return -ENOMEM;
    }

    if (cfg_param.unix_dgram_addr[0])
    {
        unlink(cfg_param.unix_dgram_addr);
        struct sockaddr_un addr;
        addr.sun_family     = AF_UNIX;
        snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", cfg_param.unix_dgram_addr);
        sock_ctx[UNIX_DGRAM_CHANNEL].fd  = CBB::cbb_sock_create(AF_UNIX, SOCK_DGRAM, 0, 1, (struct sockaddr *)&addr, sizeof(addr));
        if (sock_ctx[UNIX_DGRAM_CHANNEL].fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "create unix dgram socket failed,addr:%s\n", cfg_param.unix_dgram_addr);
            return -1;
        }

        sock_ctx[UNIX_DGRAM_CHANNEL].func        = unix_recvfrom_data;
        sock_ctx[UNIX_DGRAM_CHANNEL].context     = this;
        evt.events              = EPOLLIN;
        evt.data.ptr            = &sock_ctx[UNIX_DGRAM_CHANNEL];
        ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_ctx[UNIX_DGRAM_CHANNEL].fd, &evt);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                        ep_fd, sock_ctx[UNIX_DGRAM_CHANNEL].fd, errno);
            return -1;
        }
        CLog::log(CLog::CLOG_LEVEL_API, "[MC]create unix dgram socket successfully\n");
    }

    if (cfg_param.unix_stream_addr[0])
    {
        unlink(cfg_param.unix_stream_addr);
        struct sockaddr_un addr;
        addr.sun_family     = AF_UNIX;
        snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", cfg_param.unix_stream_addr);
        sock_ctx[UNIX_STREAM_CHANNEL].fd  = CBB::cbb_sock_create(AF_UNIX, SOCK_STREAM, 0, 1, (struct sockaddr *)&addr, sizeof(addr));
        if (sock_ctx[UNIX_STREAM_CHANNEL].fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "create unix stream socket failed,addr:%s\n", cfg_param.unix_stream_addr);
            return -1;
        }

        ret = listen(sock_ctx[UNIX_STREAM_CHANNEL].fd, 10);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "unix stream socket listen failed,errno:%d\n", errno);
            return -1;
        }


        sock_ctx[UNIX_STREAM_CHANNEL].func        = unix_new_connection;
        sock_ctx[UNIX_STREAM_CHANNEL].context     = this;
        evt.events              = EPOLLIN;
        evt.data.ptr            = &sock_ctx[UNIX_STREAM_CHANNEL];
        ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_ctx[UNIX_STREAM_CHANNEL].fd, &evt);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                        ep_fd, sock_ctx[UNIX_STREAM_CHANNEL].fd, errno);
            return -1;
        }
        CLog::log(CLog::CLOG_LEVEL_API, "[MC]create unix stream socket successfully\n");
    }

    if (cfg_param.ipv4_stream_port)
    {
        struct sockaddr_in addr;
        addr.sin_family         = AF_INET;
        addr.sin_addr.s_addr    = inet_addr(cfg_param.ipv4_stream_addr); 
        addr.sin_port           = htons(cfg_param.ipv4_stream_port);
        sock_ctx[IPV4_STREAM_CHANNEL].fd  = CBB::cbb_sock_create(AF_INET, SOCK_STREAM, 0, 1, (struct sockaddr *)&addr, sizeof(addr));
        if (sock_ctx[IPV4_STREAM_CHANNEL].fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "create ipv4 stream socket failed,addr:%s,port:%u\n",
                        cfg_param.ipv4_stream_addr, cfg_param.ipv4_stream_port);
            return -1;
        }

        ret = listen(sock_ctx[IPV4_STREAM_CHANNEL].fd, 10);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "ipv4 stream socket listen failed,errno:%d\n", errno);
            return -1;
        }

        sock_ctx[IPV4_STREAM_CHANNEL].func        = unix_new_connection;
        sock_ctx[IPV4_STREAM_CHANNEL].context     = this;
        evt.events              = EPOLLIN;
        evt.data.ptr            = &sock_ctx[IPV4_STREAM_CHANNEL];
        ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_ctx[IPV4_STREAM_CHANNEL].fd, &evt);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                        ep_fd, sock_ctx[IPV4_STREAM_CHANNEL].fd, errno);
            return -1;
        }
        CLog::log(CLog::CLOG_LEVEL_API, "[MC]create ipv4 stream socket successfully\n");
    }

    if (cfg_param.channel_vtdu_fd > 0)
    {
        sock_ctx[UNIX_VTDU_CHANNEL].func        = vtdu_recv_data;
        sock_ctx[UNIX_VTDU_CHANNEL].context     = this;
        sock_ctx[UNIX_VTDU_CHANNEL].fd          = cfg_param.channel_vtdu_fd;
        evt.events              = EPOLLIN;
        evt.data.ptr            = &sock_ctx[UNIX_VTDU_CHANNEL];
        ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, sock_ctx[UNIX_VTDU_CHANNEL].fd, &evt);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                        ep_fd, sock_ctx[UNIX_VTDU_CHANNEL].fd, errno);
            return -1;
        }
        CLog::log(CLog::CLOG_LEVEL_API, "[MC]vtdu communication socket add to epoll successfully:%d\n", cfg_param.channel_vtdu_fd);
    }

    return 0;
}


/// msgcenter function
namespace MC
{
    int mc_init()
    {
        return 0;
    }

    int mc_create(const CMcCfgParam &param)
    {
        return msg_server.create(param);
    }

    int mc_start()
    {
        return msg_server.start();
    }

    int mc_stop()
    {
        return msg_server.stop();
    }

    int mc_destroy()
    {
        return msg_server.destroy();
    }

    int mc_set_cb(void *context, void (*cb)(void *ctx, MN::CMsgFormat &msg, uint64_t time))
    {
        return msg_server.set_msg_cb(context, cb);
    }

    int mc_send_to_gb(void *buf, int len, void *ex_buf, int ex_len)
    {
        return msg_server.send_to_gb(buf, len, ex_buf, ex_len);
    }

    /// 发送数据给vtdu模块
    int mc_send_to_vtdu(uint32_t msg_type, void *buf, int len, void *ex_buf, int ex_len)
    {
        return msg_server.send_to_vtdu(msg_type, buf, len, ex_buf, ex_len);
    }

    CMcCfgParam::CMcCfgParam()
    {
        memset(unix_dgram_addr, 0, sizeof(unix_dgram_addr));
        memset(unix_stream_addr, 0, sizeof(unix_stream_addr));
        memset(ipv4_stream_addr, 0, sizeof(ipv4_stream_addr));
        ipv4_stream_port    = 0;      ///  消息中心服务外部通信端口
    }

}
