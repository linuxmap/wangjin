

#include "tinyxml2.h"
#ifndef _MSC_VER
#include <cstdlib>
#include <cstring>
#include <ctime>
#else
#include <time.h>
#endif
#include <iostream>

#include "gb.h"

#include "xmlparser.h"

using namespace tinyxml2;
// using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
#define xml_log(fmt, ...)  // printf("[XML_PARSE]%s %d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef _MSC_VER
#define strncasecmp  strnicmp
#endif


typedef struct tagXmlParser
{
    int id;
    XMLDocument *doc;
}TXmlParser;

/// #define XMLPARSER_MAX_NUM   8
/// static TXmlParser all_parser[XMLPARSER_MAX_NUM];

void xml_del(HXmlParser handle)
{
}


HXmlParser xml_create()
{
    xml_log("xml create, version:%s %s\n", __DATE__, __TIME__);
    XMLDocument *doc;
    try
    {
        doc = new XMLDocument;
    }
    catch (std::bad_alloc)
    {
        xml_log("xml create failed,no memory\n");
    }

    xml_log("xml create\n");
    return (HXmlParser)doc;
}

void xml_parse_notify(TManscdpXmlResult *result, tinyxml2::XMLElement *surface)
{
    tinyxml2::XMLElement *surfaceChild = surface->FirstChildElement();

    result->sub_cmd_type    = EMANSCDP_SUB_CMD_NONE;
    if (strncasecmp(surfaceChild->Name(), "CmdType", strlen("CmdType")))
    {
        xml_log("invalid name:%s\n", surfaceChild->Name());
        return;
    }

    if (NULL == surfaceChild->GetText())
    {
        xml_log("cmdtype have no text\n");
        return;
    }

    xml_log("sub cmd type:%s\n", surfaceChild->GetText());
    if (!strncasecmp(surfaceChild->GetText(), "Keepalive", strlen("Keepalive")))
    {
        result->sub_cmd_type    = EMANSCDP_SUB_CMD_KEEPALIVE;
        TNotifyKeepaliveMsg *keepalive = (TNotifyKeepaliveMsg *)malloc(sizeof(TNotifyKeepaliveMsg));
        if (NULL == keepalive)
        {
            xml_log("may be no mem\n");
            return;
        }
        memset(keepalive, 0, sizeof(*keepalive));
        result->value = keepalive;

        do {
            surfaceChild = surfaceChild->NextSiblingElement();
            if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
            {
                return;
            }

            if (!strncasecmp(surfaceChild->Name(), "SN", strlen("SN")))
            {
                keepalive->sn  = atoi(surfaceChild->GetText());
                xml_log("SN:%d\n", keepalive->sn);
            }
            else if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
            {
                snprintf(keepalive->device_id, sizeof(keepalive->device_id), "%s", surfaceChild->GetText());
                xml_log("device id:%s\n", keepalive->device_id);
            }
            else if (!strncasecmp(surfaceChild->Name(), "Info", strlen("Info")))
            {
                xml_log("info received\n");
            }
            else if (!strncasecmp(surfaceChild->Name(), "Status", strlen("Status")))
            {
                if (surfaceChild->GetText() && !strncasecmp(surfaceChild->GetText(), "OK", strlen("OK")))
                {
                    keepalive->status  = XML_MANSCDP_NOTIFY_STATUS_OK;
                }
                else
                {
                    keepalive->status  = XML_MANSCDP_NOTIFY_STATUS_NONOK;
                }
                xml_log("status received\n");
            }
            else
            {
                xml_log("not find xml item type:%s\n", surfaceChild->Name());
                return;
            }
        } while (1);
    }
}

void xml_parse_rec_list(tinyxml2::XMLElement *child, CRecordInfoItem **item)
{
    if ( NULL == child->Name() || strncasecmp(child->Name(), "Item", strlen("Item")) )
    {
        return;
    }

    if (child->NextSiblingElement())
    {
        xml_parse_rec_list(child->NextSiblingElement(), item);
    }

    tinyxml2::XMLElement *surfaceChild = child->FirstChildElement();
    if (NULL == surfaceChild || NULL == surfaceChild->Name())
    {
        xml_log("surface child invalid:%p\n", surfaceChild);
        return;
    }

    CRecordInfoItem *rec_item;
    rec_item    = (CRecordInfoItem *)malloc(sizeof(*rec_item));
    if (NULL == rec_item)
    {
        xml_log("may be no mem\n");
        return;
    }
    memset(rec_item, 0, sizeof(*rec_item));

    if (NULL == *item)
    {
        *item   = rec_item;
    }
    else
    {
        rec_item->next  = *item;
        *item           = rec_item;
    }

    xml_log("sub item name:%s\n", surfaceChild->Name());
    do {
        if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
        {
            snprintf(rec_item->device_id, sizeof(rec_item->device_id), "%s", surfaceChild->GetText());
            xml_log("device id:%s\n", rec_item->device_id);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Name", strlen("Name")))
        {
            snprintf(rec_item->name, sizeof(rec_item->name), "%s", surfaceChild->GetText());
            xml_log("name:%s\n", rec_item->name);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Address", strlen("Address")))
        {
            snprintf(rec_item->address, sizeof(rec_item->address), "%s", surfaceChild->GetText());
            xml_log("Address:%s\n", rec_item->address);
        }
        else if (!strncasecmp(surfaceChild->Name(), "FilePath", strlen("FilePath")))
        {
            snprintf(rec_item->file_path, sizeof(rec_item->file_path), "%s", surfaceChild->GetText());
            xml_log("file path:%s\n", rec_item->file_path);
        }
        else if (!strncasecmp(surfaceChild->Name(), "StartTime", strlen("StartTime")))
        {
            struct tm tm_to_make;
            memset(&tm_to_make, 0, sizeof(tm_to_make));
            sscanf(surfaceChild->GetText(), "%d-%d-%dT%d:%d:%d", &tm_to_make.tm_year, &tm_to_make.tm_mon,
                        &tm_to_make.tm_mday, &tm_to_make.tm_hour, &tm_to_make.tm_min, &tm_to_make.tm_sec);
            tm_to_make.tm_year  -= 1900;
            tm_to_make.tm_mon   -= 1;
            rec_item->start_time    = mktime(&tm_to_make);
            xml_log("start time:%d-%d-%dT%d:%d:%d\n", year, mon, day, hour, min, sec);
        }
        else if (!strncasecmp(surfaceChild->Name(), "EndTime", strlen("EndTime")))
        {
            struct tm tm_to_make;
            memset(&tm_to_make, 0, sizeof(tm_to_make));
            sscanf(surfaceChild->GetText(), "%d-%d-%dT%d:%d:%d", &tm_to_make.tm_year, &tm_to_make.tm_mon,
                        &tm_to_make.tm_mday, &tm_to_make.tm_hour, &tm_to_make.tm_min, &tm_to_make.tm_sec);
            tm_to_make.tm_year  -= 1900;
            tm_to_make.tm_mon   -= 1;
            rec_item->end_time      = mktime(&tm_to_make);
            xml_log("end time:%d-%d-%dT%d:%d:%d\n", year, mon, day, hour, min, sec);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Secrecy", strlen("Secrecy")))
        {
            if (NULL != surfaceChild->GetText())
            {
                rec_item->secrecy = atoi(surfaceChild->GetText());
                xml_log("secrecy:%d\n", rec_item->safe_way);
            }
        }
        else if (!strncasecmp(surfaceChild->Name(), "Type", strlen("Type")))
        {
            if (!strncasecmp(surfaceChild->GetText(), "time", strlen("time")))
            {
                rec_item->type  = CRecordInfoItem::ERECORD_TYPE_TIME;
            }
            else if (!strncasecmp(surfaceChild->GetText(), "alarm", strlen("alarm")))
            {
                rec_item->type  = CRecordInfoItem::ERECORD_TYPE_ALARM;
            }
            else if (!strncasecmp(surfaceChild->GetText(), "all", strlen("all")))
            {
                rec_item->type  = CRecordInfoItem::ERECORD_TYPE_ALL;
            }

            xml_log("record type:%d\n", rec_item->type);
        }
        else if (!strncasecmp(surfaceChild->Name(), "RecorderID", strlen("RecorderID")))
        {
            snprintf(rec_item->recorder_id, sizeof(rec_item->recorder_id), "%s", surfaceChild->GetText());
            xml_log("recorder id:%s\n", rec_item->recorder_id);
        }
        else
        {
            xml_log("not find xml item type:%s\n", surfaceChild->Name());
        }

        surfaceChild = surfaceChild->NextSiblingElement();

        if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
        {
            break;
        }

    } while (1);
}

void xml_parse_dev_list(tinyxml2::XMLElement *child, TQueryCatalogItem **item)
{
    tinyxml2::XMLElement *surfaceChild = child->FirstChildElement();
    TQueryCatalogItem *cata_log_item;

    if ( NULL == child->Name() || strncasecmp(child->Name(), "Item", strlen("Item")) )
    {
        return;
    }

    if (child->NextSiblingElement())
    {
        xml_parse_dev_list(child->NextSiblingElement(), item);
    }

    if (NULL == surfaceChild || NULL == surfaceChild->Name())
    {
        xml_log("surface child invalid:%p\n", surfaceChild);
        return;
    }

    cata_log_item = (TQueryCatalogItem *)malloc(sizeof(*cata_log_item));
    if (NULL == cata_log_item)
    {
        xml_log("may be no mem\n");
        return;
    }
    memset(cata_log_item, 0, sizeof(*cata_log_item));

    if (NULL == *item)
    {
        *item   = cata_log_item;
    }
    else
    {
        cata_log_item->next = *item;
        *item               = cata_log_item;
    }

    xml_log("sub item name:%s\n", surfaceChild->Name());
    do {
        if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
        {
            snprintf(cata_log_item->device_id, sizeof(cata_log_item->device_id), "%s", surfaceChild->GetText());
            xml_log("device id:%s\n", cata_log_item->device_id);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Name", strlen("Name")))
        {
            snprintf(cata_log_item->name, sizeof(cata_log_item->name), "%s", surfaceChild->GetText());
            xml_log("name:%s\n", cata_log_item->name);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Manufacturer", strlen("Manufacturer")))
        {
            snprintf(cata_log_item->manufacturer, sizeof(cata_log_item->manufacturer), "%s", surfaceChild->GetText());
            xml_log("manufacturer:%s\n", cata_log_item->manufacturer);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Model", strlen("Model")))
        {
            snprintf(cata_log_item->model, sizeof(cata_log_item->model), "%s", surfaceChild->GetText());
            xml_log("Model:%s\n", cata_log_item->model);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Owner", strlen("Owner")))
        {
            snprintf(cata_log_item->owner, sizeof(cata_log_item->owner), "%s", surfaceChild->GetText());
            xml_log("Owner:%s\n", cata_log_item->owner);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Civilcode", strlen("Civilcode")))
        {
            snprintf(cata_log_item->civil_code, sizeof(cata_log_item->civil_code), "%s", surfaceChild->GetText());
            xml_log("Civilcode:%s\n", cata_log_item->civil_code);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Address", strlen("Address")))
        {
            snprintf(cata_log_item->address, sizeof(cata_log_item->address), "%s", surfaceChild->GetText());
            xml_log("Address:%s\n", cata_log_item->address);
        }
        else if (!strncasecmp(surfaceChild->Name(), "SafetyWay", strlen("SafetyWay")))
        {
            if (NULL != surfaceChild->GetText())
            {
                cata_log_item->safe_way = atoi(surfaceChild->GetText());
                xml_log("SafetyWay:%d\n", cata_log_item->safe_way);
            }
        }
        else if (!strncasecmp(surfaceChild->Name(), "RegisterWay", strlen("RegisterWay")))
        {
            if (NULL != surfaceChild->GetText())
            {
                cata_log_item->register_way = atoi(surfaceChild->GetText());
                xml_log("RegisterWay:%d\n", cata_log_item->safe_way);
            }
        }
        else if (!strncasecmp(surfaceChild->Name(), "Secrecy", strlen("Secrecy")))
        {
            if (NULL != surfaceChild->GetText())
            {
                cata_log_item->secrecy = atoi(surfaceChild->GetText());
                xml_log("secrecy:%d\n", cata_log_item->safe_way);
            }
        }
        else if (!strncasecmp(surfaceChild->Name(), "Status", strlen("Status")))
        {
            if (surfaceChild->GetText() && !strncasecmp(surfaceChild->GetText(), "ON", strlen("ON")))
            {
                cata_log_item->status  = XML_MANSCDP_NOTIFY_STATUS_OK;
            }
            else
            {
                cata_log_item->status  = XML_MANSCDP_NOTIFY_STATUS_NONOK;
            }
            xml_log("status received,value:%d\n", cata_log_item->status);
        }
        else
        {
            xml_log("not find xml item type:%s\n", surfaceChild->Name());
        }

        surfaceChild = surfaceChild->NextSiblingElement();

        if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
        {
            break;
        }

    } while (1);
}

/*
 * parse QUERY record info xml
 */
void xml_parse_query_record_info(TManscdpXmlResult *result, tinyxml2::XMLElement *surfaceChild)
{
    GB::CGbRecordItem *query_info = (GB::CGbRecordItem *)malloc(sizeof(GB::CGbRecordItem));
    if (NULL == query_info)
    {
        xml_log("may be no memory\n");
        return;
    }
    memset(query_info, 0, sizeof(GB::CGbRecordItem));

    result->sub_cmd_type    = EMANSCDP_SUB_CMD_RECORD_INFO;
    result->value           = query_info;
    while(true)
    {
        surfaceChild = surfaceChild->NextSiblingElement();
        if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
        {
            break;
        }

        if (!strncasecmp(surfaceChild->Name(), "SN", strlen("SN")))
        {
            query_info->sn  = atoi(surfaceChild->GetText());
            xml_log("SN:%d\n", query_info->sn);
        }
        else if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
        {
            if (surfaceChild->GetText())
            {
                snprintf(query_info->device_id, sizeof(query_info->device_id), "%s", surfaceChild->GetText());
            }
            xml_log("device id:%s\n", query_info->device_id);
        }
        else if (!strncasecmp(surfaceChild->Name(), "StartTime", strlen("StartTime")))
        {
            struct tm tm_to_make;
            memset(&tm_to_make, 0, sizeof(tm_to_make));
            sscanf(surfaceChild->GetText(), "%d-%d-%dT%d:%d:%d", &tm_to_make.tm_year, &tm_to_make.tm_mon,
                        &tm_to_make.tm_mday, &tm_to_make.tm_hour, &tm_to_make.tm_min, &tm_to_make.tm_sec);
            tm_to_make.tm_year  -= 1900;
            tm_to_make.tm_mon   -= 1;
            query_info->start_time  = mktime(&tm_to_make);
            xml_log("query record info,start time:%d-%d-%dT%d:%d:%d\n", year, mon, day, hour, min, sec);
        }
        else if (!strncasecmp(surfaceChild->Name(), "EndTime", strlen("EndTime")))
        {
            struct tm tm_to_make;
            memset(&tm_to_make, 0, sizeof(tm_to_make));
            sscanf(surfaceChild->GetText(), "%d-%d-%dT%d:%d:%d", &tm_to_make.tm_year, &tm_to_make.tm_mon,
                        &tm_to_make.tm_mday, &tm_to_make.tm_hour, &tm_to_make.tm_min, &tm_to_make.tm_sec);
            tm_to_make.tm_year  -= 1900;
            tm_to_make.tm_mon   -= 1;
            query_info->end_time    = mktime(&tm_to_make);
            xml_log("query record info,start time:%d-%d-%dT%d:%d:%d\n", year, mon, day, hour, min, sec);
        }
        else if (!strncasecmp(surfaceChild->Name(), "FilePath", strlen("FilePath")))
        {
            snprintf(query_info->file_path, sizeof(query_info->file_path), "%s", surfaceChild->GetText());
            xml_log("file path:%s\n", query_info->path);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Address", strlen("Address")))
        {
            snprintf(query_info->address, sizeof(query_info->address), "%s", surfaceChild->GetText());
            xml_log("address:%s\n", query_info->address);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Secrecy", strlen("Secrecy")))
        {
            query_info->secrecy = atoi(surfaceChild->GetText());
            xml_log("secrecy:%d\n", query_info->secrecy);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Type", strlen("Type")))
        {
            if ( surfaceChild->GetText() && (strncasecmp(surfaceChild->GetText(), "time", sizeof("time"))) ) 
            {
                query_info->type = GB::EGBRECORD_TYPE_TIME;
            }
            else if ( surfaceChild->GetText() && (strncasecmp(surfaceChild->GetText(), "alarm", sizeof("alarm"))) )
            {
                query_info->type = GB::EGBRECORD_TYPE_ALARM;
            }
            else if ( surfaceChild->GetText() && (strncasecmp(surfaceChild->GetText(), "manual", sizeof("manual"))) )
            {
                query_info->type = GB::EGBRECORD_TYPE_MANUAL;
            }
            else if ( surfaceChild->GetText() && (strncasecmp(surfaceChild->GetText(), "all", sizeof("all"))) )
            {
                query_info->type = GB::EGBRECORD_TYPE_ALL;
            }

            xml_log("record type:%s\n", query_info->type);
        }
        else if (!strncasecmp(surfaceChild->Name(), "RecorderID", strlen("RecorderID")))
        {
            snprintf(query_info->recorder_id, sizeof(query_info->recorder_id), "%s", surfaceChild->GetText());
            xml_log("recorder id:%s\n", query_info->recorder_id);
        }
        else if (!strncasecmp(surfaceChild->Name(), "Indistinct", strlen("Indistinct")))
        {
            query_info->indistinct  = atoi(surfaceChild->GetText());
            xml_log("indistinct :%d\n", query_info->recorder_id);
        }
        else
        {
            xml_log("Query::catalog:not find xml item type:%s\n", surfaceChild->Name());
        }
    }
}

/// parse query
void xml_parse_query(TManscdpXmlResult *result, tinyxml2::XMLElement *surface)
{
    tinyxml2::XMLElement *surfaceChild = surface->FirstChildElement();
    result->sub_cmd_type    = EMANSCDP_SUB_CMD_NONE;

    if (strncasecmp(surfaceChild->Name(), "CmdType", strlen("CmdType")))
    {
        xml_log("invalid name:%s\n", surfaceChild->Name());
        return;
    }

    if (NULL == surfaceChild->GetText())
    {
        xml_log("cmdtype:%s have no text\n", surfaceChild->Name());
        return;
    }
    xml_log("sub cmd type:%s\n", surfaceChild->GetText());

    if (!strncasecmp(surfaceChild->GetText(), "Catalog", strlen("Catalog")))
    {
        result->sub_cmd_type    = EMANSCDP_SUB_CMD_CATALOG;
        while(true)
        {
            surfaceChild = surfaceChild->NextSiblingElement();
            if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
            {
                break;
            }

            if (!strncasecmp(surfaceChild->Name(), "SN", strlen("SN")))
            {
                result->sn  = atoi(surfaceChild->GetText());
                xml_log("SN:%d\n", result->sn);
            }
            else if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
            {
                snprintf(result->device_id, sizeof(result->device_id), "%s", surfaceChild->GetText());
                xml_log("device id:%s\n", result->device_id);
            }
            else
            {
                xml_log("Query::catalog:not find xml item type:%s\n", surfaceChild->Name());
            }
        }
    }
    else if (!strncasecmp(surfaceChild->GetText(), "DeviceInfo", strlen("DeviceInfo")))
    {
        result->sub_cmd_type    = EMANSCDP_SUB_CMD_DEVICE_INFO;
        while(true)
        {
            surfaceChild = surfaceChild->NextSiblingElement();
            if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
            {
                break;
            }

            if (!strncasecmp(surfaceChild->Name(), "SN", strlen("SN")))
            {
                result->sn  = atoi(surfaceChild->GetText());
                xml_log("Query DevInfo SN:%d\n", result->sn);
            }
            else if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
            {
                snprintf(result->device_id, sizeof(result->device_id), "%s", surfaceChild->GetText());
                xml_log("Query DevInfo id:%s\n", result->device_id);
            }
            else
            {
                xml_log("Query::DevInfo:not find xml item type:%s\n", surfaceChild->Name());
            }
        }
    }
    else if (!strncasecmp(surfaceChild->GetText(), "RecordInfo", strlen("RecordInfo")))
    {
        xml_parse_query_record_info(result, surfaceChild);
    }
}

void xml_parse_res(TManscdpXmlResult *result, tinyxml2::XMLElement *surface)
{
    tinyxml2::XMLElement *surfaceChild = surface->FirstChildElement();

    result->sub_cmd_type    = EMANSCDP_SUB_CMD_NONE;
    if (strncasecmp(surfaceChild->Name(), "CmdType", strlen("CmdType")))
    {
        xml_log("invalid name:%s\n", surfaceChild->Name());
        return;
    }

    if (NULL == surfaceChild->GetText())
    {
        xml_log("cmdtype have no text\n");
        return;
    }

    xml_log("sub cmd type:%s\n", surfaceChild->GetText());
    if (!strncasecmp(surfaceChild->GetText(), "Catalog", strlen("Catalog")))
    {
        result->sub_cmd_type    = EMANSCDP_SUB_CMD_CATALOG;
        TManscdpQueryCatalogRes *cata_log = (TManscdpQueryCatalogRes *)malloc(sizeof(TManscdpQueryCatalogRes));
        if (NULL == cata_log)
        {
            xml_log("may be no mem\n");
            return;
        }
        memset(cata_log, 0, sizeof(*cata_log));
        result->value = cata_log;

        do {
            surfaceChild = surfaceChild->NextSiblingElement();
            if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
            {
                return;
            }

            if (!strncasecmp(surfaceChild->Name(), "SN", strlen("SN")))
            {
                cata_log->sn  = atoi(surfaceChild->GetText());
                xml_log("SN:%d\n", cata_log->sn);
            }
            else if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
            {
                snprintf(cata_log->device_id, sizeof(cata_log->device_id), "%s", surfaceChild->GetText());
                xml_log("device id:%s\n", cata_log->device_id);
            }
            else if (!strncasecmp(surfaceChild->Name(), "SumNum", strlen("SumNum")))
            {
                cata_log->sum_num = atoi(surfaceChild->GetText());
                xml_log("sum num:%d\n", cata_log->sum_num);
            }
            else if (!strncasecmp(surfaceChild->Name(), "DeviceList", strlen("DeviceList")))
            {
                const char *p = strstr(surfaceChild->Name(), "=");
                if (p)
                {
                    cata_log->dev_list_num = atoi(++p);
                }

                tinyxml2::XMLElement *child = surfaceChild->FirstChildElement();
                if (NULL != child)
                {
                    cata_log->item  = NULL;
                    xml_parse_dev_list(child, &cata_log->item);
                }

                xml_log("status received\n");
            }
            else
            {
                xml_log("not find xml item type:%s\n", surfaceChild->Name());
                return;
            }
        } while (1);

    }
    else if (!strncasecmp(surfaceChild->GetText(), "RecordInfo", strlen("RecordInfo")))
    {
        result->sub_cmd_type    = EMANSCDP_SUB_CMD_RECORD_INFO;
        CManscdpResRecordInfo *record_info = (CManscdpResRecordInfo *)malloc(sizeof(CManscdpResRecordInfo));
        if (NULL == record_info)
        {
            xml_log("may be no mem\n");
            return;
        }
        memset(record_info, 0, sizeof(*record_info));
        result->value = record_info;

        do {
            surfaceChild = surfaceChild->NextSiblingElement();
            if ( (NULL == surfaceChild) || (NULL == surfaceChild->Name()) )
            {
                return;
            }

            if (!strncasecmp(surfaceChild->Name(), "SN", strlen("SN")))
            {
                record_info->sn  = atoi(surfaceChild->GetText());
                xml_log("SN:%d\n", record_info->sn);
            }
            else if (!strncasecmp(surfaceChild->Name(), "DeviceID", strlen("DeviceID")))
            {
                snprintf(record_info->device_id, sizeof(record_info->device_id), "%s", surfaceChild->GetText());
                xml_log("device id:%s\n", record_info->device_id);
            }
            else if (!strncasecmp(surfaceChild->Name(), "SumNum", strlen("SumNum")))
            {
                record_info->sum_num = atoi(surfaceChild->GetText());
                xml_log("sum num:%d\n", record_info->sum_num);
            }
            else if (!strncasecmp(surfaceChild->Name(), "Name", strlen("Name")))
            {
                snprintf(record_info->name, sizeof(record_info->name), "%s", surfaceChild->GetText());
                xml_log("device id:%s\n", record_info->device_id);
            }
            else if (!strncasecmp(surfaceChild->Name(), "RecordList", strlen("RecordList")))
            {
                const char *p = strstr(surfaceChild->Name(), "\"");
                if (p)
                {
                    record_info->dev_list_num = atoi(++p);
                }

                tinyxml2::XMLElement *child = surfaceChild->FirstChildElement();
                if (NULL != child)
                {
                    record_info->item  = NULL;
                    xml_parse_rec_list(child, &record_info->item);
                }

                xml_log("status received\n");
            }
            else
            {
                xml_log("not find xml item type:%s\n", surfaceChild->Name());
                continue;
            }
        } while (1);
    }

}

void xml_parse_text(HXmlParser handle, char *text, TManscdpXmlResult **xml_result)
{
    TManscdpXmlResult *result;
    XMLDocument *doc = static_cast<XMLDocument *>(handle);
    doc->Clear();
    doc->Parse(text);
    XMLElement *scene = doc->RootElement();

    *xml_result = NULL;

    // result->manscdp_type    = EMANSCDP_TYPE_NONE;

    if ((NULL == scene)|| (NULL == scene->Name()))
    {
        xml_log("fatal error,find no xml root element\n");
        return;
    }

    result = (TManscdpXmlResult *)malloc(sizeof(*result));
    if (NULL == result)
    {
        xml_log("xml parser,fatal error,no mem\n");
    }
    memset(result, 0, sizeof(*result));

    if (!strncasecmp(scene->Name(), "Notify", strlen("Notify")))
    {
        result->manscdp_type    = EMANSCDP_TYPE_NOTIFY;
    }
    else if (!strncasecmp(scene->Name(), "Query", strlen("Query")))
    {
        result->manscdp_type    = EMANSCDP_TYPE_QUERY;
    }
    else if (!strncasecmp(scene->Name(), "Response", strlen("Response")))
    {
        result->manscdp_type    = EMANSCDP_TYPE_RESPONSE;
    }
    else if (!strncasecmp(scene->Name(), "Control", strlen("Control")))
    {
        result->manscdp_type    = EMANSCDP_TYPE_CONTROL;
    }
    else
    {
        xml_log("not find manscdp type:%s\n", scene->Name());
        free(result);
        return;
    }

    xml_log("manscdp type:%d\n", result->manscdp_type);
    switch (result->manscdp_type)
    {
        case EMANSCDP_TYPE_NOTIFY:
            xml_parse_notify(result, scene);
            break;

        case EMANSCDP_TYPE_QUERY:
            xml_parse_query(result, scene);
            break;

        case EMANSCDP_TYPE_RESPONSE:
            xml_parse_res(result, scene);
            break;

        case EMANSCDP_TYPE_CONTROL:
            break;

        default:
            free(result);
            return;
    }
    *xml_result = result;
}

void xml_free_manscdp_result(TManscdpXmlResult *result)
{
    if (NULL == result)
    {
        return;
    }

    switch (result->manscdp_type)
    {
        case EMANSCDP_TYPE_CONTROL:
            break;

        case EMANSCDP_TYPE_QUERY:
            switch (result->sub_cmd_type)
            {
                case EMANSCDP_SUB_CMD_KEEPALIVE:
                    free(result->value);
                    break;

                case EMANSCDP_SUB_CMD_RECORD_INFO:
                    if (result->value)
                    {
                        free(result->value);
                    }
                    break;

                default:
                    free(result->value);
                    xml_log("xml free manscep result, invalid subcmd type:%d\n", result->sub_cmd_type);
                    break;
            }

            break;

        case EMANSCDP_TYPE_NOTIFY:
            switch (result->sub_cmd_type)
            {
                case EMANSCDP_SUB_CMD_KEEPALIVE:
                    if (result->value)
                    {
                        free(result->value);
                    }
                    break;

                default:
                    if (result->value)
                    {
                        free(result->value);
                    }
                    xml_log("xml free manscep result, invalid subcmd type:%d\n", result->sub_cmd_type);
                    break;
            }
            break;

        case EMANSCDP_TYPE_RESPONSE:
            switch (result->sub_cmd_type)
            {
                case EMANSCDP_SUB_CMD_CATALOG:
                    {
                        TManscdpQueryCatalogRes *cata_log = (TManscdpQueryCatalogRes *)result->value;
                        if (cata_log)
                        {
                            TQueryCatalogItem *item = cata_log->item;
                            TQueryCatalogItem *p = item;
                            while (item)
                            {
                                p = item->next;
                                free(item);
                                item = p;
                            }
                        }
                        free(result->value);
                    }
                    break;
                case EMANSCDP_SUB_CMD_RECORD_INFO:
                    {
                        CManscdpResRecordInfo *rec_info = (CManscdpResRecordInfo *)result->value;
                        if (rec_info)
                        {
                            CRecordInfoItem *item   = rec_info->item;
                            CRecordInfoItem *p      = item;
                            while (item)
                            {
                                p = item->next;
                                free(item);
                                item = p;
                            }
                        }
                        free(result->value);
                    }
                    break;

                default:
                    free(result->value);
                    xml_log("xml free manscep result, invalid subcmd type:%d\n", result->sub_cmd_type);
                    break;
            }
            break;

        default:
            xml_log("xml free manscep result, invalid manscdp type:%d\n", result->manscdp_type);
            break;
    }

    free(result);
}

int xml_copy_manscdp_result(TManscdpXmlResult *dst_result, TManscdpXmlResult *src_result)
{
    xml_free_manscdp_result(dst_result);

    switch (src_result->manscdp_type)
    {
        case EMANSCDP_TYPE_CONTROL:
            break;

        case EMANSCDP_TYPE_QUERY:
            break;

        case EMANSCDP_TYPE_NOTIFY:
            break;

        case EMANSCDP_TYPE_RESPONSE:
            break;

        default:
            xml_log("update xml result, invalid manscdp type:%d\n", src_result->manscdp_type);
            break;
    }

    return 0;
}



#ifdef __cplusplus
}
#endif
