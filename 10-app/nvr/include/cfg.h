#ifndef __CFG_H__
#define __CFG_H__

#include <stdint.h>

namespace CFG
{

/// NVS全局配置文件,所有端口皆为主机字节序
struct CCfgParam
{
    /// 构造函数
    CCfgParam();
    
    void show();

    /// DEBUG param
    uint16_t    dbg_port;                   /// 调试端口
    uint32_t    dbg_max_reg_func_num;       /// 调试模块支持的最大命令数

    /// DEV param
    uint32_t    dev_max_dev_num;                /// 最大注册设备数量
    uint32_t    dev_max_item_num;               /// 最大可用资源条目数量

    /// VTDU param
    char        vtdu_media_recv_ip[16];             /// 媒体接收地址
    

    /// msg center param
    char        mc_unix_dgram_addr[64];     ///  消息中心服务内部通信地址
    char        mc_unix_stream_addr[64];     ///  消息中心服务内部通信地址
    char        mc_ipv4_stream_addr[16];  ///  消息中心服务外部通信地址
    uint16_t    mc_ipv4_stream_port;      ///  消息中心服务外部通信端口

    /// GB28181 stack server param
    char        gbs_ip[16];             /// 国标服务地址
    uint16_t    gbs_port;               /// 国标服务端口

    char        gbs_sip_id[64];         /// server GB id
    char        gbs_domain_id[64];
    char        gbs_user_agent[64];

    char        gbs_inter_addr[64];     /// 国标服务内部通信地址
    char        gbs_external_addr[16];  /// 国标服务外部通信地址
    uint16_t    gbs_external_port;      /// 国标服务外部通信端口

    char        gbs_ncproxy_addr[16];   /// 国标服务客户端通信地址
    uint16_t    gbs_ncproxy_port;       /// 国标服务客户端通信端口

    uint32_t    gbs_max_reg_dev_num;    /// 国标服务器最大接入设备数
    uint32_t    gbs_max_res_num;        /// 国标服务器最大资源条目数目(目录查询条目)

    uint32_t    gbs_session_linger_time;    /// 会话保持时间
};




/// 读取配置文件
int cfg_init(char *cfg_file, CCfgParam &param);

/// 加载默认配置参数
void cfg_load_dft(CCfgParam  &nvs_cfg_param);

}



#endif  /// __CFG_H__

