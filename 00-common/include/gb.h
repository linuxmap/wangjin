#ifndef __GB_H__
#define __GB_H__

#include <stdint.h>



#ifdef _MSC_VER
// 预定义宏

#define GB_API __declspec(dllexport)

#else

#define GB_API __attribute__ ((visibility("default")))
#endif
#include "msgno.h"
typedef void* gb_handle_t;


#define GBSIP_USERNAME_LEN      32
#define GBSIP_MAX_DEVICE_NUM    32
#define GBSIP_USER_AGENT_LEN    32
#define GBSIP_VIA_HOST_LEN      64
#define GBSIP_VIA_PORT_LEN      8
#define GBSIP_GENERAL_ITEM_LEN  32

#define GBSIP_MANSCDP_DEVICE_ID_MAX_SIZE 64

/// 国标错误码
#define GB_SUCCESS          0
#define GB_ERR_BASE         10000
#define GB_ERR_NOMEM        (10000 + 1)     /// 内存耗尽
#define GB_ERR_ACCESS       (10000 + 2)     /// 非法操作
#define GB_ERR_ARG          (10000 + 3)     /// 非法参数
#define GB_ERR_BUSY         (10000 + 4)     /// 忙
#define GB_ERR_AGAIN        (10000 + 5)     /// 暂时不可获取



/// GB协议栈接口参数涉及到ip地址、端口信息统一为主机字节序
namespace GB
{
    /// 国标服务配置参数
    struct GB_API CGbSvrParam
    {
        CGbSvrParam();

        /// GB28181 stack server param
        char        ip[16];             /// 国标服务地址
        uint16_t    port;               /// 国标服务端口

        char        sip_id[64];         /// server GB id
        char        domain_id[64];
        char        user_agent[64];

        char        inter_addr[64];     /// 国标服务内部通信地址
        char        external_addr[16];  /// 国标服务外部通信地址
        uint16_t    external_port;      /// 国标服务外部通信端口

        char        ncproxy_addr[16];   /// 国标服务客户端通信地址
        uint16_t    ncproxy_port;       /// 国标服务客户端通信端口

        uint32_t    max_reg_dev_num;    /// 国标服务器最大接入设备数
        uint32_t    max_res_num;        /// 国标服务器最大资源条目数目(目录查询条目)

        uint32_t    reg_period;             /// 回话注册时间
        uint32_t    session_linger_time;    /// 会话保持时间,单位:ms
        bool        manual_start;           /// 若为true,需要调用gb_server_start才能开启服务
    };

    /// 国标客户端配置参数,该结构中所有端口参数为主机序
    struct GB_API CGbClientParam
    {
        CGbClientParam() {};

        char        ip[16];             /// 国标客户端地址
        uint16_t    port;               /// 国标客户端端口

        char        sip_id[64];         /// client GB id
        char        domain_id[64];
        char        user_agent[64];

        char        inter_addr[64];     /// 国标客户端内部通信地址
        char        external_addr[16];  /// 国标客户端外部通信地址
        uint16_t    external_port;      /// 国标客户端外部通信端口

        uint32_t    session_linger_time;    /// 会话保持时间,单位:ms
    };

    /// 国标manscdp配置参数
    struct GB_API CGbManscdpParam
    {
        CGbManscdpParam();
        char                    device_id[GBSIP_MANSCDP_DEVICE_ID_MAX_SIZE];    /// 必选项 设备编码
        char                    domain_id[GBSIP_MANSCDP_DEVICE_ID_MAX_SIZE];    /// 域编码
        char                    *user_agent;                                    /// 用户代理名称
        char                    *device_name;                                   /// 必选 设备名称
        char                    *manufacturer;      /// 必选 设备厂商
        char                    *model;             /// 必选 设备型号
        char                    *owner;             /// 必选 设备归属
        char                    *civil_code;        /// 必选 行政区域
        char                    *address;           /// 必选 安装地址
        bool                    parental;           /// 必选 是否有子设备
        bool                    on;                 /// 必选 设备状态
    };


    class GB_API CGbDev         // uac device
    {
    public:
        CGbDev();
        ~CGbDev();

    public:
        char                    dev_id[GBSIP_USERNAME_LEN];
        char                    dev_domain_id[GBSIP_USERNAME_LEN];

        uint32_t                src_host;       // network order
        uint16_t                src_port;       // network order

        uint64_t                last_update_time;                   // millisecond

    };

    class GB_API CGbItem               // gb resouces
    {
    public:
        CGbItem();
        ~CGbItem();

        int request_stream();
        int stop_stream();

    private:
        char    device_id[GBSIP_USERNAME_LEN];
        char    name[GBSIP_USERNAME_LEN];

        char    manufacturer[GBSIP_GENERAL_ITEM_LEN];
        char    model[GBSIP_GENERAL_ITEM_LEN];
        char    owner[GBSIP_GENERAL_ITEM_LEN];
        char    civil_code[GBSIP_GENERAL_ITEM_LEN];
        char    address[GBSIP_GENERAL_ITEM_LEN];
        int     parental;
        int     safe_way;
        int     register_way;
        int     secrecy;
        int     status;
        CGbDev  *dev;
    };

    /// GB28181 define
    enum EGbRecordType
    {
        EGBRECORD_TYPE_NULL,
        EGBRECORD_TYPE_TIME,
        EGBRECORD_TYPE_ALARM,
        EGBRECORD_TYPE_MANUAL,
        EGBRECORD_TYPE_ALL,
    };

    /// GB28181 record query infomation
    struct CGbRecordQueryParam
    {
        char    dev_id[GBSIP_USERNAME_LEN];
        int64_t    start_time;                         /// number of seconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC)
        int64_t    end_time;                           /// @start_time
        int     type;                               /// @EGbRecordType,default:0
        int     secrecy;                            /// default:0
        int     indistinct;                         /// optional
        char    address[GBSIP_GENERAL_ITEM_LEN];
        void    *user_ctx;                          /// user data
    };

    /// record info item def
    struct CGbRecordItem
    {
        /// all item info
        char        parent_dev_id[GBSIP_USERNAME_LEN];          /// 父设备id
        int         sn;                                         /// 本次操作对应的manscdp的SN
        uint32_t    sum_num;                                    /// 目录信息的总条目数
        char        parent_name[GBSIP_GENERAL_ITEM_LEN];        /// 父设备的名字
        void        *user_ctx;                                  /// 用户上下文

        /// current item info
        char        device_id[GBSIP_USERNAME_LEN];          /// 设备ID
        char        name[GBSIP_GENERAL_ITEM_LEN];           /// 设备名字
        char        file_path[GBSIP_GENERAL_ITEM_LEN];      /// 文件路径
        char        address[GBSIP_GENERAL_ITEM_LEN];        /// 地址，看厂家的实现
        int64_t     start_time;                             /// 开始时间
        int64_t     end_time;                               /// 结束时间
        int         secrecy;                                /// 是否涉密
        int         type;                                   /// 录像产生类型,optional,@EGbRecordType
        char        recorder_id[GBSIP_USERNAME_LEN];        /// optional
        int         indistinct;                             /// optional
        CGbRecordItem* next;
    };


#ifndef _MSC_VER
    // gb init function
    GB_API int gb_init();

    GB_API int gb_server_create(gb_handle_t *handle, CGbSvrParam &param);

    GB_API int gb_server_start(gb_handle_t handle);

    GB_API int gb_server_set_cb(gb_handle_t handle, void *context, int(*cb)(void *ctx, int type, void *var, int var_len));

    GB_API int gb_server_invite(gb_handle_t handle, MN::CMediaInvite &invite_param);

    GB_API int gb_server_reponse_catalog(gb_handle_t handle, MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len);

    GB_API int gb_server_rmt_invite(gb_handle_t handle, MN::CMediaInvite &invite_param);
#endif

    /// 国标客户端接口
    GB_API int gb_client_create(gb_handle_t *handle, CGbClientParam &param);

    GB_API int gb_client_start(gb_handle_t handle);

    struct GB_API CGbRegParam
    {
        char        server_sip_id[21];
        char        server_domain_id[21];
        char        passwd[32];
        uint32_t    server_ip;
        uint16_t    server_port;
        uint32_t    keep_interval;                  /// 心跳间隔
        uint32_t    keep_max_cnt;                   /// 最大心跳次数,超过该次数,链路将断开
    };
    GB_API int gb_client_reg_to_server(gb_handle_t handle, const CGbRegParam &reg_param);

    GB_API int gb_client_setparam(gb_handle_t handle, CGbClientParam &param);

    GB_API int gb_client_getparam(gb_handle_t handle, CGbClientParam &param);

    enum client_evt_t
    {
        ECLIENT_EVT_REGISTERED,             /// 成功注册,无参数
        ECLIENT_EVT_RES_CATALOG,            /// 目录结构应答,参数是:@CGbCatalogItem

        ECLIENT_EVT_RES_INVITE,            /// media invite response,param:@CGbInviteMediaRes
        ECLIENT_EVT_RES_QUERY_RECORD_INFO,  /// query record infomation response,param:@CGbRecordItem
    };

    struct GB_API CGbInviteMediaRes
    {
        uint16_t    media_send_port;
        uint8_t     ptype;
        uint32_t    ssrc;
    };

    /// 目录查询结果:资源条目
    struct GB_API CGbCatalogItem
    {
        char    device_id[21];
        char    parent_dev_id[32];
        char    name[64];

        char    manufacturer[64];
        char    model[64];
        char    owner[64];
        char    civil_code[64];
        char    address[64];
        int     parental;
        int     safe_way;
        int     register_way;
        int     secrecy;
        int     status;
    };

    /// set callback
    GB_API int gb_client_set_cb(gb_handle_t handle, void *context, int (*cb)(void *ctx, int type, void *var, int var_len));

    /// query catalog
    GB_API int gb_client_query_catalog(gb_handle_t handle);

    /* query record info 
     * @secrecy:0 or 1
     */
    GB_API int gb_client_query_record_info(gb_handle_t handle, CGbRecordQueryParam &query_param );

    //GB_API int gb_client_query_record_info(gb_handle_t handle, char *dev_id, int secrecy,
    //                                        EGbRecordType type, long start_time, long end_time, char *address, void *ctx);

    /// request one media
    GB_API int gb_client_invite_media(gb_handle_t handle, char *media_sender_id,
                                        uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype);

    /// close one media
    GB_API int gb_client_bye_media(gb_handle_t handle, char *media_sender_id,
                                        uint32_t ssrc, uint16_t media_recv_port, uint8_t ptype);

    GB_API int gb_client_stop(gb_handle_t handle);
}



#endif  // __GB_H__
