#ifndef __DEV_API_H__
#define __DEV_API_H__

/// c++ std interface
#include <iostream>

/// 消息中心头文件
#include "msgno.h"


namespace DEV 
{


struct CDevRecordItem;


struct CDevAsyncMsg
{
    uint32_t    msg_type;
    uint32_t    from_addr;                   /// 0:gb_server 1:gb_client
    uint32_t    ctx;
    uint64_t    start_time;
    uint64_t    last_update_time;
    uint64_t    timeout_time;
    void        *req_data;
    size_t      req_len;

    void        *response;
    size_t      res_len;
    CDevAsyncMsg    *next;
};


/// inter class
class CResouceItem
{
public:
    /// 初始化
    static int init(uint32_t max_item_num, uint32_t media_recv_ip);

    /// add one catalog from ResoureCenter
    static int add(MN::CCatalogItem &cata_log);

    /// delete one catalog from ResoureCenter
    static int del(MN::CCatalogItem &cata_log);

    /// 调度函数
    static void run(uint64_t cur_time);

    /// 执行媒体请求操作
    static void invite_media();

    /// 查询设备录像信息
    static void query_record_info(uint64_t cut_time);

    /// 显示所有条目
    static void show_all_item(int (*pf)(const char *fmt, ...) = NULL);

    /// 执行目录查询功能
    static int query_cata_log(MN::CRmtQueryCatalog &query_catalog_param);

    static int rmt_invite_media(MN::CMediaInvite &invite_param);

    static int rmt_ack_media(MN::CMediaInvite &invite_param);

    /// close media channel
    static int rmt_bye_media(MN::CMediaInvite &invite_param);

    /*
     * add rec info
     */
    static int add_rec_info(MN::CMsgGbResRecordInfo &rec_info, MN::CMsgGbRecordItem *rec_item, uint64_t time);

    /// remote query record info
    static int query_record_info(MN::CMsgGbQueryRecordInfo &query_param, uint64_t time);


    static uint32_t get_async_msg_ctx();

    CResouceItem();

private:
    bool    valid;
    bool    play;
    uint32_t    channel_id;         /// 对应VTDU通道号

    enum EMediaSessionNum
    {
        EMEDIA_SESSION_VIDEO    = 0,
        EMEDIA_SESSION_AUDIO    = 1,
        EMEDIA_SESSION_META     = 2,
        EMEDIA_SESSION_CALLBACK = 3,

        /// add new session here
        
        EMEDIA_SESSION_NUM      = 4,
    };
    MN::CNetSession local_addr[EMEDIA_SESSION_NUM];

    std::string  device_id;
    std::string  name;
    std::string  manufacturer;
    std::string  model;
    std::string  owner;
    std::string  civil_code;
    std::string  address;
    int     parental;
    int     safe_way;
    int     register_way;
    int     secrecy;
    bool    on; 
    uint32_t    ip;
    uint16_t    port;

    std::string         parent_device_id;        // can be none

    /// 媒体发送组信息,发送组统计信息需要由VTDU实时上报
    struct CMediaTransmitter
    {
        uint32_t    to_ip;              /// 网络序
        uint32_t    spy_ip;             /// 网络序
        uint16_t    to_port;            /// 主机序
        uint16_t    spy_port;           /// 主机序
        uint32_t    ssrc;               /// 转发通道SSRC值(为0保持)
        uint8_t     ptype;              /// 载荷类型

        uint64_t    start_time;         /// 开始时间
        uint64_t    last_update_time;   /// 最近一次更新时间
        uint64_t    snd_packets;        /// 累计发送包数
        uint64_t    snd_error_packets;  /// 累计发送错误包数
        uint64_t    snd_bytes;          /// 发送字节数
        uint64_t    snd_error_bytes;    /// 累计发送字节数

        uint32_t    item_id;            /// 条目ID
        uint32_t    dev_id;             /// 设备ID
        uint32_t    dlg_id;             /// 会话ID
        uint32_t    transsmit_id;       /// 发送器ID
        uint32_t    vtdu_ch_id;         /// vtdu 通道ID

        bool        start;              /// 是否开始生效
        uint64_t    last_time;          /// 上一次更新时间

        CMediaTransmitter   *next;

        CMediaTransmitter();

        void init_data();

        /// 初始化
        static void init(CMediaTransmitter *list_hdr, uint32_t list_size);

        /// 获取一个节点
        static CMediaTransmitter *get_one_node(CMediaTransmitter **list_hdr);

        /// 增加一个节点
        static void add_one_node(CMediaTransmitter **list_hdr, CMediaTransmitter *node);

        /// give up one node to the free table
        static void giveback_one_node(CMediaTransmitter **list_hdr, CMediaTransmitter *node);
    }*transmitter;
    static CMediaTransmitter   *transmitter_table_unused;      /// 媒体发送列表
    static CMediaTransmitter   *transmitter_table_start;      /// 媒体发送列表

    /// record query
    struct CDevRecQueryResult
    {
        int                         start_time;
        int                         update_time;
        int                         rec_sum_num;
        int                         rec_cur_num;

        CDevRecordItem              *rec_list;          /// record list item
    };


    /// max item num and cur valid item at runtime
    static uint32_t max_item_num;
    static uint32_t valid_item_num;

    /// 有效发送数量可以设置为条目数的2倍
    static uint32_t valid_tramsmitter_num;

    /// 本地媒体接收端口起始地址
    static uint16_t LOCAL_MEDIA_RECV_BASE_PORT;
    static uint32_t LOCAL_MEDIA_RECV_ADDR;

    /// log handler
    static int (*log)(const char* format, ...);

    /// send buffer
    static char send_buf[8192];
    static int  send_buf_len;

private:
    /// print item info
    void show(int(*pf)(const char *fmt, ...) = NULL);

    static void deal_rec_list(MN::CMsgGbQueryRecordInfo &rec_req_info, MN::CMsgGbResRecordInfo &rec_res_info, CDevRecQueryResult *result);

    /*
     * on success, return zero,on error, return nonzero
     */
    static int get_item_by_id(char *dst_id, uint32_t *index = NULL);

    /*
     * on success, return zero,on error, return nonzero
     */
    static int get_async_msg_by_ctx(uint32_t ctx, CDevAsyncMsg **dst_msg);

    /*
     * add rec item to result list
     */
    static int add_rec_item_to_result(CDevRecQueryResult *result, MN::CMsgGbRecordItem *rec_item, int num, uint64_t time);

    /*
     * free aync rec query msg
     */
    static void del_rec_info(CDevRecQueryResult *result, CDevAsyncMsg *async_msg);
public:
    /// asynchronous msg queue
    static uint32_t             async_msg_ctx;
    static CDevAsyncMsg         *async_msg_queue;
    static uint32_t             async_msg_size;
};

void *dev_malloc(size_t size);
void *dev_mallocz(size_t size);
void dev_free(void *p);

}
#endif  // __DEV_API_H__


