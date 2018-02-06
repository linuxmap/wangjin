

#ifdef _MSC_VER

#else
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#define WSAGetLastError()       errno
#endif

#include "common.h"
#include "link.h"

#ifndef _MSC_VER
void CLink::deal_sock()
{
}
#else
void CLink::deal_sock()
{
    int ret, addr_len;
    struct sockaddr_in addr;
    static fd_set read_set, write_set, error_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&error_set);

    if (INVALID_SOCKET != sip_fd)
    {
        FD_SET(sip_fd, &read_set);
    }

    if (INVALID_SOCKET != inter_fd)
    {
        FD_SET(inter_fd, &read_set);
    }

    if (INVALID_SOCKET != external_fd)
    {
        FD_SET(external_fd, &read_set);
    }

    struct timeval tv;
    tv.tv_sec   = timeout / 1000;
    tv.tv_usec  = 1000 * (timeout % 1000);
    ret = select(0, &read_set, &write_set, &error_set, &tv);
    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "select failed\n");
        return;
    }

    SYSTEMTIME Systime;
    GetLocalTime(&Systime);
    cur_time = time(NULL) * 1000;
    cur_time += Systime.wMilliseconds;

    if ( (INVALID_SOCKET != sip_fd) && FD_ISSET(sip_fd, &read_set) )
    {
        /// 处理sip信令
        addr_len = sizeof(addr);
        ret = CBB::cbb_recvfrom(sip_fd, recv_buf, sizeof(recv_buf), &addr, &addr_len, 0);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "recvfrom failed:%d\n", WSAGetLastError());
        }
        else
        {
            /// callback sip msg
            if (NULL != sip_data_cb)
            {
                sip_data_cb(sip_cb_ctx, recv_buf, ret, addr.sin_addr.s_addr, addr.sin_port, cur_time);
            }
        }
    }

    if ((INVALID_SOCKET != inter_fd) && FD_ISSET(inter_fd, &read_set))
    {
        /// 处理内部消息
    }

    if ((INVALID_SOCKET != external_fd) && FD_ISSET(external_fd, &read_set))
    {
        /// 处理外部命令
    }
}
#endif

/*
 * create sock linker
 */
int CLink::create(CCfgParam &param)
{
    int ret;
    struct sockaddr_in addr;

    if (0 != param.client_port)
    {
        addr.sin_family         = AF_INET;
        addr.sin_addr.s_addr    = param.client_addr;
        addr.sin_port           = param.client_port;
        sip_fd                  = CBB::cbb_sock_create(AF_INET, SOCK_DGRAM, 0, 1, &addr, sizeof(addr));
        if (sip_fd < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "gblink create socket failed\n");
            sip_fd = INVALID_SOCKET;
            return -1;
        }

        ret = CBB::cbb_socket_nonblock(sip_fd, 1);
        if (ret < 0)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "gblink set socket nonblock failed:%d\n", WSAGetLastError());
        }

        int nBufSize = 2 * 1024 * 1024;
        ret = setsockopt(sip_fd, SOL_SOCKET, SO_RCVBUF, (char *)&nBufSize, sizeof(nBufSize));
        gb_write("setsockopt recv buf ret:%d\n", ret);
    }

    sip_data_cb = param.client_data_cb;
    sip_cb_ctx  = param.client_data_cb_ctx;

    try
    {
        link_task_handle = new std::thread(sock_routine, this);
    }
    catch (...)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gbclient create thread failed\n");
        CBB::cbb_sock_close(sip_fd);
        return -1;
    }

    return 0;
}

/// routine
void *CLink::sock_routine(void *arg)
{
    CLink *link = (CLink *)arg;         // cur link

    for (;;)
    {
        link->deal_sock();

        link->deal_trigger();
    }

    return NULL;
}

/*
 * deal epoll trigger
 */
void CLink::deal_trigger()
{
    trigger_count++;

    if (trigger_cb)
    {
        trigger_cb(trigger_ctx, cur_time);
    }
}

/// 国标服务发送消息
int CLink::sip_send_data(void *buf, int len, uint32_t rmt_ip, uint16_t rmt_port)
{
    struct sockaddr_in  addr;
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = rmt_ip;
    addr.sin_port           = rmt_port;
    uint32_t addr_len       = sizeof(addr);

    return CBB::cbb_sendto(sip_fd, buf, len, &addr, addr_len, 0);
}


/// 构造函数
CLink::CLink()
{
    /// thread module
    link_task_handle    = NULL;

    sip_fd              = INVALID_SOCKET;
    inter_fd            = INVALID_SOCKET;
    external_fd         = INVALID_SOCKET;

    timeout             = 1000;
    sip_data_cb         = NULL;        /// sip消息回调函数
    sip_cb_ctx          = NULL;
    trigger_cb          = NULL;
    trigger_ctx         = NULL;
}
