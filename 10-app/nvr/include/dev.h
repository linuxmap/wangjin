#ifndef __DEV_H__
#define __DEV_H__

#include <stdint.h>

#include "msgno.h"

#define DEV_API __attribute__ ((visibility("default")))

namespace DEV 
{
    /// 设备中心初始化
    struct DEV_API CDevCfgParam
    {
        CDevCfgParam();

        uint32_t    max_item_num;       /// default:128
        uint32_t    max_dev_num;        /// default:128
        uint32_t    media_recv_addr;    /// IP地址,网络序
    };

    // gb init function
    DEV_API int dev_init(CDevCfgParam &cfg_param);

    DEV_API int dev_add(MN::CDevRegister &dev_reg);

    DEV_API int dev_del(MN::CDevRegister &dev_reg);

    DEV_API int cata_log_add(MN::CCatalogItem &cata_log);

    DEV_API int cata_log_del(MN::CCatalogItem &cata_log);

    DEV_API int query_cata_log(MN::CRmtQueryCatalog &query_catalog_param);

    DEV_API int rmt_invite_media(MN::CMediaInvite &invite_param);

    DEV_API int rmt_ack_media(MN::CMediaInvite &invite_param);

    DEV_API int rmt_bye_media(MN::CMediaInvite &invite_param);

    DEV_API int add_rec_info(MN::CMsgGbResRecordInfo &rec_info, MN::CMsgGbRecordItem *rec_item, uint64_t time);

    /// remote query record info
    DEV_API int rmt_query_record_info(MN::CMsgGbQueryRecordInfo &query_param, uint64_t time);
}


#endif  // __DEV_H__


