#ifndef __ITEM_REC_H__
#define __ITEM_REC_H__

#include <stdint.h>

/// 消息中心头文件
#include "msgno.h"

namespace DEV
{

struct CDevRecordItem
{
    /// GB28181 define
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
    int         type;                           /// optional,@ERecordType
    char        recorder_id[MN_DEV_ID_LEN];     /// optional

    uint64_t    update_time;
    bool        used;
    CDevRecordItem *next;
    CDevRecordItem ();

    bool        is_same(MN::CMsgGbRecordItem &item, uint64_t time);
    bool        set_to_value(MN::CMsgGbRecordItem &item, uint64_t time);

    static bool create(uint32_t num = 1024);
    static bool get_one_node(CDevRecordItem **item);
    static void giveback_one_node(CDevRecordItem *item);

private:
    static CDevRecordItem *all_list;     
    static CDevRecordItem *free_list;     
    static uint32_t     item_num;
};

}


#endif  /// __ITEM_REC_H__
