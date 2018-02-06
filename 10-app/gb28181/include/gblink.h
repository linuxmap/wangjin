
#ifndef __GB_LINK_H__
#define __GB_LINK_H__

#include <stdint.h>

class CGbSockLink; 

class CGbLink
{
public:
    typedef void (*FInetDataCb)(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t cur_time);
    typedef void (*FUnixDataCb)(void *context, void *buf, int buf_len, char *rmt_addr, uint64_t cur_time);

    typedef void (*FEvenTrigger)(void *context, uint64_t cur_time);

    struct CCfgParam
    { 
        CCfgParam();
        uint32_t        svr_addr;               /// 国标服务地址
        uint16_t        svr_port;               /// 国标服务端口
        FInetDataCb     gb_svr_data_cb;         /// 国标回调
        void            *gb_svr_data_cb_ctx;    /// 国标回调用户上下文

        char            inter_msg_addr[64];     /// 内部通信端口,进程内部使用UNIX域UDP通信方式
        FUnixDataCb     inter_msg_cb;           /// 内部消息回调
        void            *inter_msg_cb_ctx;      /// 内部消息回调用户上下文

        uint32_t        external_addr;          /// 国标服务器配置信令通信地址
        uint16_t        external_port;          /// 国标服务器配置信令通信端口
        FInetDataCb     external_data_cb;       /// 国标回调
        void            *external_data_cb_ctx;  /// 国标回调用户上下文

        uint32_t        ncproxy_addr;           /// NC客户端地址
        uint16_t        ncproxy_port;           /// NC客户端端口
        FInetDataCb     ncproxy_data_cb;        /// 国标回调
        void            *ncproxy_data_cb_ctx;   /// 国标回调用户上下文

        FEvenTrigger    event_trigger_cb;       /// 触发器
        void            *ev_trigger_ctx;        /// 触发器用户上下文
    };

    /// 构造函数
    CGbLink();

    /*
     * create sock linker
     */
    int create(CCfgParam &param);

    /// 国标服务发送消息
    int svr_send_data(void *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /// 内部通信发送消息
    int inter_send_data(char *buf, int len, char *rmt_addr);

    /// 配置信令通道发送消息
    int external_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /// 客户端通道发送消息
    int nc_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /*
     * destroy link
     */
    int destroy();

private:
    CGbSockLink         *sock_link;             /// 套接字链路层
};


#endif  // __GB_LINK_H__
