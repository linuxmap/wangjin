
#ifndef __GB_SIP_H__
#define __GB_SIP_H__


#define GB_MANSCDP_DEVICE_ID_MAX_SIZE   64
/* 注册方式 */
typedef enum
{
    GB_MANSCDP_REGISTERWAY_SIP                  = 1,    /// 默认值 符合sip3261标准的认证注册模式
    GB_MANSCDP_REGISTERWAY_PASSWORD,            /// 2   /// 基于口令的双向认证注册模式
    GB_MANSCDP_REGISTERWAY_DIGITAL_CERTIFICATE, /// 3   /// 基于数字证书的双向认证注册模式
}EGbRegisterWay;

typedef struct tagGbSipInitParam
{
    char                    device_id[GB_MANSCDP_DEVICE_ID_MAX_SIZE];   /// 必选 设备编码
    char*                   szName;         /// 必选 设备名称
    char*                   szManufacturer; /// 必选 设备厂商
    char*                   szCatalogType;  /// 目录类型    0  固定监控点   1   可控监控点 90~99  预留 
    char*                   szRecLocation;  /// 存储类型    0  前端设备存储 1  集中存储服务器存储 2   无存储计划 90~99  预留
    char*                   szOperateType;  /// 操作类型    MOD    修改  ADD    添加    DEL    删除
    char*                   szModel;        /// 必选 设备型号
    char*                   szOwner;        /// 必选 设备归属
    char*                   szCivilCode;    /// 必选 行政区域
    char*                   szBlock;        /// 可选 警区
    char*                   szAddress;      /// 必选 安装地址
    char*                   szParentID;     /// 可选 父设备ID 有父设备需要填写
    char*                   szBusinessGroupID; /// 可选 虚拟组织所属的业务分组ID，上报虚拟组织目录项的时候使用
    EGbRegisterWay         *peRegisterWay;   /// 必选 注册方式
    uint16_t                port;                
}TGbSipInitParam;

typedef void (*FGblinkDataCb)(void *context, void *buf, int buf_len, uint32_t src_ip, uint16_t port);

/*
 * create sock linker
 */
int gblink_init();

void gblink_set_data_cb(FGblinkDataCb cb, void *user_context);

/*
 * gblink send data to rmt
 */
int gblink_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

#endif  // __GB_SIP_H__
