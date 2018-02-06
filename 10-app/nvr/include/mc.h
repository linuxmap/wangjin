#ifndef __MSG_CENTER_H__
#define __MSG_CENTER_H__

#include "msgno.h"

#define MC_API __attribute__ ((visibility("default")))

namespace MC
{
    /// 消息中心服务配置参数
    struct MC_API CMcCfgParam
    {
        CMcCfgParam();

        char        unix_dgram_addr[64];     ///  消息中心服务内部通信地址
        char        unix_stream_addr[64];     ///  消息中心服务内部通信地址

        char        ipv4_stream_addr[16];  ///  消息中心服务外部通信地址
        uint16_t    ipv4_stream_port;      ///  消息中心服务外部通信端口

        int         channel_vtdu_fd;
    };

    // gb init function
    MC_API int mc_init();

    MC_API int mc_create(const CMcCfgParam &param);

    MC_API int mc_start();

    MC_API int mc_send_to_gb(void *buf, int len, void *ex_buf, int ex_len);

    MC_API int mc_send_to_vtdu(uint32_t msg_type, void *buf, int len, void *ex_buf, int ex_len);

    MC_API int mc_set_cb(void *context, void (*cb)(void *ctx, MN::CMsgFormat &msg, uint64_t time));

    MC_API int mc_stop();

    MC_API int mc_destroy();
}


#endif      // __MSG_CENTER_H__
