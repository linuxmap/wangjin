
#ifndef __MSG_NUMBER_H__
#define __MSG_NUMBER_H__

#include <stdint.h>

// 预定义宏
#ifdef _MSC_VER
#define MN_API __declspec(dllexport)

#else

#define MN_API __attribute__ ((visibility("default")))
#endif

// gb2818 related
#define MN_GB_DEV_REGISTERED            2000     // param type:CDevRegister
#define MN_GB_CATALOG_RES               2001        // 
#define MN_GB_INVITE_MEDIA              2002
#define MN_GB_RMT_QUERY_CATALOG         2003     // rmt server query catalog,param type:CRmtQueryCatalog
#define MN_GB_LOCAL_RES_CATALOG         2004     // local server response catalog
#define MN_GB_RMT_INVITE_MEDIA          2005        // rmt uac invite media,param type:CMediaInvite 
#define MN_GB_RMT_ACK_MEDIA             2006        // rmt server ACK invite,param type:CMediaInvite
#define MN_GB_RMT_BYE_MEDIA             2007        /// rmt sip client bye media,param type:CMediaInvite

#define MN_GB_LOCAL_QUERY_RECORDINFO    2008        /// nvs core request gbstack to query recordinfo,param type:CMsgGbQueryRecordInfo
#define MN_GB_RMT_RES_RECORDINFO        2009        /// rmt dev response record info request,param type:CMsgGbRecordItem

#define MN_GB_RMT_QUERY_RECORDINFO      2010        /// nvs core request gbstack to query recordinfo,param type:CMsgGbQueryRecordInfo
#define MN_GB_LOCAL_RES_RECORDINFO      2011        /// rmt dev response record info request,param type:CMsgGbRecordItem


/// VTDU related
#define MN_VTDU_ADD_SWITCH_IPV4         5000     // 增加转发:@CMsgAddSwitchIpv4
#define MN_VTDU_DEL_SWITCH_IPV4         5001     // 删除转发:@CMsgAddSwitchIpv4


#define MN_DEV_ID_LEN       32
#define MN_DEV_NAME_LEN     32 
#define MN_GENERAL_ITEM_LEN 32

namespace MN
{
    // msg format,host byte-order
    struct MN_API CMsgFormat
    {
        int             type;
        unsigned int    sn;
        uint32_t        op_result;
        int             var_len;
        void            *var;
    };

    struct MN_API CDevRegister
    {
        CDevRegister();
        char    device_id[MN_DEV_ID_LEN];
        char    name[MN_DEV_ID_LEN];
        char    ip[MN_GENERAL_ITEM_LEN];
        char    port[MN_GENERAL_ITEM_LEN];
        int     sum_item_num;
    };

    struct MN_API CCatalogItem
    {
        CCatalogItem();

        char    device_id[MN_DEV_ID_LEN];
        char    name[MN_DEV_ID_LEN];
        char    manufacturer[MN_GENERAL_ITEM_LEN];
        char    model[MN_GENERAL_ITEM_LEN];
        char    owner[MN_GENERAL_ITEM_LEN];
        char    civil_code[MN_GENERAL_ITEM_LEN];
        char    address[MN_GENERAL_ITEM_LEN];
        int     parental;
        int     safe_way;
        int     register_way;
        int     secrecy;
        bool    on; 
        uint32_t    ip;
        uint16_t    port;

        char    parent_device_id[MN_DEV_ID_LEN];        // can be none
    };

    /** 网络参数*/
    struct MN_API CNetSession
    {
        CNetSession();

        // stream info
        uint16_t    rtp_port;
        uint32_t    rtp_ip;
        uint16_t    rtcp_port;
        uint32_t    rtcp_ip;
    };

    struct MN_API CMediaInvite
    {
        CMediaInvite();

        char        device_id[MN_DEV_ID_LEN];
        CNetSession local_net_info;
        CNetSession rmt_rtp_addr;
        uint32_t    dev_ip;
        uint16_t    dev_port;
        uint32_t    ssrc;
        uint32_t    item_id;
        uint32_t    dev_id;
        uint32_t    dlg_id;
        uint32_t    transsmit_id;
        uint8_t     payload_type;

        void        *ext_data;
    };

    struct MN_API CRmtQueryCatalog
    {
        CRmtQueryCatalog();

        char        device_id[MN_DEV_ID_LEN];
        uint32_t    rmt_ip;
        uint16_t    rmt_port;
        uint16_t    num;
        int         sn;
    };

}


namespace MN 
{
/***************************************************************************
    /// msg transport over UDP,TCP...,all fields in network byte order 
         magic              len
     +++++++++++++++++++++++++++++++++++
    |      16       |       16          |
     +++++++++++++++++++++++++++++++++++

       ver        crc         seq
     +++++++++ +++++++++ ++++++++++++++++
    |    8    |    8    |       16       |
     +++++++++ +++++++++ ++++++++++++++++

                  cmd type
     +++++++++ +++++++++ ++++++++++++++++
    |               32                   |
     +++++++++ +++++++++ ++++++++++++++++
***************************************************************************/

    /// 流式消息头部标记,占用４字节
     struct MN_API CStreamHdr
     {
         uint16_t   magic;          /// 流式协议头部标记:$@,0x24 0x40
         uint16_t   len;            
         static void buf2hdr(uint8_t *buf, CStreamHdr &str_hdr);

         static void hdr2buf(uint8_t *buf, CStreamHdr &str_hdr);
     };

/// VER:1
#define MC_MSG_PROTOCOL_VER01       (uint8_t(1))

    /// 消息头部定义
    struct MN_API CMsgHdr
    {
        uint8_t     ver;    /// 当前版本采用0xFF,后续版本累减
        uint8_t     crc;    /// 头部字段校验和,计算该值时,crc值取0
        uint16_t    seq;    /// 序列号,发送端关心字段,接收端原值返回
        uint32_t    type;   /// 消息类型

        CMsgHdr(uint16_t _seq, uint32_t _type);

        CMsgHdr();


        static void buf2hdr(uint8_t *buf, CMsgHdr &msg_hdr);

        static bool crc_match(uint8_t *buf);

        static uint8_t tb_crc8(uint8_t *data, int len);

        /// buf must greater than 8 bytes
        static bool msg2buf_calc_crc(uint8_t *buf, uint32_t &buf_len, CMsgHdr &msg_hdr);
    };


    /// Ipv4增加转发组参数
    struct MN_API CMsgAddSwitchIpv4
    {
        static bool buf2msg(uint8_t *buf, uint32_t len, CMsgAddSwitchIpv4 &msg);

        static bool msg2buf(uint8_t *buf, uint32_t &buf_len, CMsgAddSwitchIpv4 &msg);

        uint32_t    channel_id;     /// 接收组ID
        uint32_t    to_ip;          /// 网络序
        uint32_t    spy_ip;         /// 网络序
        uint16_t    to_port;        /// 主机序
        uint16_t    spy_port;       /// 主机序
        uint32_t    ssrc;           /// 转发通道SSRC值(为0保持)
        uint32_t    op_result;      /// 操作结果
    };

    struct MN_API CMsgGbQueryRecordInfo
    {
        static bool buf2msg(uint8_t *buf, uint32_t len, CMsgAddSwitchIpv4 &msg);

        static bool msg2buf(uint8_t *buf, uint32_t &buf_len, CMsgAddSwitchIpv4 &msg);

        CMsgGbQueryRecordInfo();

        char    device_id[MN_DEV_ID_LEN];
        char    catalog_id[MN_DEV_ID_LEN];
        char    recorder_id[MN_DEV_ID_LEN];
        int64_t start_time;
        int64_t end_time;

        int     type;                               /// @EGbRecordType,default:0
        int     secrecy;                            /// default:0
        char    address[MN_GENERAL_ITEM_LEN];
        int     indistinct;                         /// optional

        uint32_t rmt_ip;
        uint16_t rmt_port;

        uint64_t ctx;                   /// user data
    };

    struct MN_API CMsgGbResRecordInfo
    {
        static bool buf2msg(uint8_t *buf, uint32_t len, CMsgAddSwitchIpv4 &msg);

        static bool msg2buf(uint8_t *buf, uint32_t &buf_len, CMsgAddSwitchIpv4 &msg);

        CMsgGbResRecordInfo();

        char        device_id[MN_DEV_ID_LEN];
        char        name[MN_GENERAL_ITEM_LEN];
        int         sum_num;
        int         list_num;

        uint64_t    ctx;                /// user data
    };

    struct MN_API CMsgGbRecordItem
    {
        static bool buf2msg(uint8_t *buf, uint32_t len, CMsgAddSwitchIpv4 &msg);

        static bool msg2buf(uint8_t *buf, uint32_t &buf_len, CMsgAddSwitchIpv4 &msg);

        CMsgGbRecordItem();

        enum ERecordType
        {
            ERECORD_TYPE_NULL,
            ERECORD_TYPE_TIME,
            ERECORD_TYPE_ALARM,
            ERECORD_TYPE_MANUAL,
            ERECORD_TYPE_ALL,
        };
        char        device_id[MN_DEV_ID_LEN];
        char        name[MN_GENERAL_ITEM_LEN];
        char        file_path[MN_GENERAL_ITEM_LEN];
        char        address[MN_GENERAL_ITEM_LEN];
        long int    start_time;
        long int    end_time;
        int         secrecy;
        int         type;                           /// optional@ERecordType
        char        recorder_id[MN_DEV_ID_LEN];     /// optional
    };


    /// 协议解析器
    class MN_API CMsgParser
    {
    public:
        CMsgParser();                   /// 构造函数
        ~CMsgParser();                  /// 析构函数

        int create(uint32_t parser_buf_size = 2048);

        /// 消息处理句柄
        void set_msg_handler(void (*msg_deal_func)(void *ctx, CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time), void *ctx);

        /// 处理内部协议
        void parser_income_data(uint8_t *buf, uint32_t len, uint64_t cur_time);

        /// 解析协议
        int parser_protocol(uint64_t cur_time);

    private:
        uint8_t     *parser_buf;            /// 待解析的buffer
        uint32_t    valid_data_len;         /// 有效数据长度,即待解析数据长度
        uint32_t    parser_buf_len;         /// 解析buffer实际长度

        /// 用户消息处理句柄和上下文
        void        (*msg_deal_func)(void *ctx, CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time);
        void        *msg_deal_ctx;
    };

}


#endif  // __MSG_NUMBER_H__
