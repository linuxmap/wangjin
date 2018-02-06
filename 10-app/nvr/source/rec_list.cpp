
#include <stdlib.h>
#include <string.h>


#include "oscbb.h"
#include "item_rec.h"


namespace DEV
{
/// rec list num region
static const uint32_t MAX_REC_ITEM_NUM      = 4096;
static const uint32_t MIN_REC_ITEM_NUM      = 16;


/// global val
CDevRecordItem *CDevRecordItem::all_list      = NULL;
CDevRecordItem *CDevRecordItem ::free_list    = NULL;     
uint32_t    CDevRecordItem::item_num       = 1024;

/// construction
CDevRecordItem::CDevRecordItem()
{
    memset(device_id, 0, sizeof(device_id));
    memset(name, 0, sizeof(name));
    memset(file_path, 0, sizeof(file_path));
    memset(address, 0, sizeof(address));
    start_time  = 0;
    end_time    = 0;
    secrecy     = 0;
    type        = ERECORD_TYPE_NULL;                           /// optional
    memset(recorder_id, 0, sizeof(recorder_id));

    update_time = 0;
    used        = false;
    next        = NULL;
}

/// create record item list
bool CDevRecordItem::create(uint32_t num)
{
    if (NULL == all_list)
    {
        CBB_SET_LIMIT(item_num, MAX_REC_ITEM_NUM, MIN_REC_ITEM_NUM);
        try
        {
            all_list    = new CDevRecordItem [item_num];
        }
        catch (...)
        {
            item_num    = 0;
            return false;
        }

        for (uint32_t i = 0; i < item_num - 1; i++)
        {
            all_list[i].next = all_list + i + 1;
        }
        all_list[item_num - 1].next = NULL;
        free_list   = all_list;


        return true;
    }
    return false;
}

/// get one free record item
bool CDevRecordItem::get_one_node(CDevRecordItem **item)
{
    if (free_list)
    {
        *item       = free_list;
        free_list   = free_list->next;
        return true;
    }
    return false;
}

/// giveback one record item to free list
void CDevRecordItem::giveback_one_node(CDevRecordItem *item)
{
    memset(item->device_id, 0, sizeof(item->device_id));
    memset(item->name, 0, sizeof(item->name));
    memset(item->file_path, 0, sizeof(item->file_path));
    memset(item->address, 0, sizeof(item->address));
    item->start_time  = 0;
    item->end_time    = 0;
    item->secrecy     = 0;
    item->type        = ERECORD_TYPE_NULL;                           /// optional
    memset(item->recorder_id, 0, sizeof(item->recorder_id));

    item->update_time = 0;
    item->used        = false;

    item->next  = free_list;
    free_list   = item;
}

bool CDevRecordItem::is_same(MN::CMsgGbRecordItem &item, uint64_t time)
{
    if ( (0 == memcmp(device_id, item.device_id, sizeof(device_id)))
        && (memcmp(file_path, item.file_path, sizeof(file_path)))
        && (start_time == item.start_time)
        && (end_time == item.end_time) )
    {
        update_time = time;
        return true;
    }

    return false;
}

bool CDevRecordItem::set_to_value(MN::CMsgGbRecordItem &item, uint64_t time)
{
    memcpy(device_id, item.device_id, sizeof(device_id));
    memcpy(name, item.name, sizeof(name));
    memcpy(file_path, item.file_path, sizeof(file_path));
    memcpy(address, item.address, sizeof(address));
    memcpy(recorder_id, item.recorder_id, sizeof(recorder_id));
    start_time  = item.start_time;
    end_time    = item.end_time;
    secrecy     = item.secrecy;
    type        = item.type;
    update_time = time;
    used        = true;
    return true;
}

}
