
#ifndef __LINK_H__
#define __LINK_H__

#include "oscbb.h"
#include <stdint.h>
#include <thread>

class CLink
{
public:
    typedef void(*FInetDataCb)(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t cur_time);
    typedef void(*FUnixDataCb)(void *context, void *buf, int buf_len, char *rmt_addr, uint64_t cur_time);

    typedef void(*FEvenTrigger)(void *context, uint64_t cur_time);

    /// 本结构体中网络地址都为网络序
    struct CCfgParam
    {
        CCfgParam(){};
        uint32_t        client_addr;               /// 国标客户端地址
        uint16_t        client_port;               /// 国标客户端端口
        FInetDataCb     client_data_cb;         /// 国标回调
        void            *client_data_cb_ctx;    /// 国标回调用户上下文
#if 0
        char            inter_msg_addr[64];     /// 内部通信端口,进程内部使用UNIX域UDP通信方式
        FUnixDataCb     inter_msg_cb;           /// 内部消息回调
        void            *inter_msg_cb_ctx;      /// 内部消息回调用户上下文

        uint32_t        external_addr;          /// 国标服务器配置信令通信地址
        uint16_t        external_port;          /// 国标服务器配置信令通信端口
        FInetDataCb     external_data_cb;       /// 国标回调
        void            *external_data_cb_ctx;  /// 国标回调用户上下文
#endif

        FEvenTrigger    event_trigger_cb;       /// 触发器
        void            *ev_trigger_ctx;        /// 触发器用户上下文
    };

    /// 构造函数
    CLink();

    /*
    * create sock linker
    */
    int create(CCfgParam &param);

    /// 国标服务发送消息
    int sip_send_data(void *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /// 内部通信发送消息
    int inter_send_data(char *buf, int len, char *rmt_addr);

    /// 配置信令通道发送消息
    int external_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /*
    * destroy link
    */
    int destroy();

    /// routine
    static void *sock_routine(void *arg);

private:

    void deal_trigger();

    void deal_sock();

    /// thread module
    std::thread     *link_task_handle;

    SOCKHANDLE		 sip_fd;
    SOCKHANDLE		 inter_fd;
    SOCKHANDLE		external_fd;

    unsigned int    timeout;        /// 单位:ms
    uint64_t        cur_time;

    char            recv_buf[8192];     /// 网络接收buffer

    FInetDataCb     sip_data_cb;            /// sip消息回调函数
    void            *sip_cb_ctx;            /// sip消息回调用户数据
    FEvenTrigger    trigger_cb;             /// 事件触发器
    void            *trigger_ctx;           /// 触发上下文
    uint64_t        trigger_count;
};


#endif // __LINK_H__
