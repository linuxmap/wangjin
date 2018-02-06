
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "common.h"
#include "mc.h"
#include "dev.h"
#include "nvs_core.h"

#define mn_log(fmt, ...)  printf("[MN]%s %d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)




void CNvsCore::deal_msg(void *ctx, MN::CMsgFormat &msg, uint64_t time)
{
    int ret = -1;
    switch (msg.type)
    {
        case MN_GB_DEV_REGISTERED:
            if (msg.var_len == sizeof(MN::CDevRegister))
            {
                printf("dev registered\n");
                MN::CDevRegister dev_reg;
                memcpy(&dev_reg, msg.var, sizeof(dev_reg));
                ret = DEV::dev_add(dev_reg);
            }
            break;

        case MN_GB_CATALOG_RES:
            if (msg.var_len == sizeof(MN::CCatalogItem))
            {
                printf("catalog add msg received\n");
                MN::CCatalogItem    item;
                memcpy(&item, msg.var, sizeof(item));
                ret = DEV::cata_log_add(item);
            }
            else
            {
                printf("msg len error\n");
            }
            break;

        case MN_GB_RMT_QUERY_CATALOG:
            if (msg.var_len == sizeof(MN::CRmtQueryCatalog))
            {
                printf("catalog add msg received\n");
                MN::CRmtQueryCatalog query_catalog_param;
                memcpy(&query_catalog_param, msg.var, sizeof(query_catalog_param));
                ret = DEV::query_cata_log(query_catalog_param);
            }
            else
            {
                mn_log("msg len error, type:%d\n", msg.type);
            }
            break;

        case MN_GB_RMT_INVITE_MEDIA:
            if (msg.var_len == sizeof(MN::CMediaInvite))
            {
                printf("media invite msg received\n");
                MN::CMediaInvite invite_param;
                memcpy(&invite_param, msg.var, sizeof(invite_param));
                ret = DEV::rmt_invite_media(invite_param);
            }
            else
            {
                mn_log("msg len error, type:%d\n", msg.type);
            }
            break;

        case MN_GB_RMT_ACK_MEDIA:
            if (msg.var_len == sizeof(MN::CMediaInvite))
            {
                printf("media ack msg received\n");
                MN::CMediaInvite invite_param;
                memcpy(&invite_param, msg.var, sizeof(invite_param));
                ret = DEV::rmt_ack_media(invite_param);
            }
            else
            {
                mn_log("msg len error, type:%d\n", msg.type);
            }
            break;

        case MN_GB_RMT_BYE_MEDIA:
            if (msg.var_len == sizeof(MN::CMediaInvite))
            {
                mn_log("media ack msg received\n");
                MN::CMediaInvite invite_param;
                memcpy(&invite_param, msg.var, sizeof(invite_param));
                ret = DEV::rmt_bye_media(invite_param);
            }
            else
            {
                mn_log("msg len error, type:%d\n", msg.type);
            }
            break;

        case MN_GB_RMT_RES_RECORDINFO:
            ret = deal_rmt_res_recordinfo(ctx, msg, time);
            break;

        case MN_GB_RMT_QUERY_RECORDINFO:
            if (msg.var_len == sizeof(MN::CMsgGbQueryRecordInfo))
            {
                mn_log("remote query record infomation\n");

                MN::CMsgGbQueryRecordInfo   query_param;
                memcpy(&query_param, msg.var, sizeof(query_param));
                ret = DEV::rmt_query_record_info(query_param, time);
            }
            else
            {
                mn_log("msg len error, type:%d\n", msg.type);
            }
            break;

        default:
            break;
    }

    if (0 != ret)
    {
        mn_log("msg center deal msg failed,msg type:%d, ret:%d\n", msg.type, ret);
    }
}

int CNvsCore::deal_rmt_res_recordinfo(void *ctx, MN::CMsgFormat &msg, uint64_t time)
{
    if (msg.var_len < (int)sizeof(MN::CMsgGbResRecordInfo))
    {
        mn_log("msg len error, type:%d,len:%d\n", msg.type, msg.var_len);
        return -EINVAL;
    }

    int ret;
    MN::CMsgGbRecordItem *rec_item = NULL;
    MN::CMsgGbResRecordInfo rec_info;
    memcpy(&rec_info, msg.var, sizeof(rec_info));
    int tmp = msg.var_len - (int)sizeof(MN::CMsgGbResRecordInfo);
    if (tmp != rec_info.list_num * (int)sizeof(MN::CMsgGbRecordItem))
    {
        mn_log("record response len invalid, tmp:%d,var len:%d,list num:%d,sizeof item:%lu\n",
                    tmp, msg.var_len, rec_info.list_num, sizeof(MN::CMsgGbRecordItem));
        return -EINVAL;
    }

    mn_log("deal record info,dev id:%s,name:%s,sum num:%d,list num:%d\n",
                rec_info.device_id, rec_info.name, rec_info.sum_num, rec_info.list_num);
    if (0 != rec_info.list_num)
    {
        rec_item    = (MN::CMsgGbRecordItem *)((uint8_t *)msg.var + sizeof(rec_info));
    }
    ret = DEV::add_rec_info(rec_info, rec_item, time);
    if (0 != ret)
    {
        mn_log("call DEV::rmt_res_rec_info failed, ret:%d\n", ret);
        return -EINVAL;
    }
    return 0;
}

