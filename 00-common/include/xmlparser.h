#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
// 预定义宏
#define TINYXML2_API __declspec(dllexport)
#else
#define TINYXML2_API __attribute__ ((visibility("default")))
#endif


#define XMLPARSER_INVALID_HANDLE    NULL
typedef void *      HXmlParser;


typedef enum tagManscdpTYpe
{
    EMANSCDP_TYPE_NONE = 0,
    EMANSCDP_TYPE_CONTROL,
    EMANSCDP_TYPE_QUERY,
    EMANSCDP_TYPE_NOTIFY,
    EMANSCDP_TYPE_RESPONSE,
}EMsgManscdpYpe;

typedef enum tagManscdpcmdType
{
    EMANSCDP_SUB_CMD_NONE= 0,
    EMANSCDP_SUB_CMD_KEEPALIVE,          /// 保活
    EMANSCDP_SUB_CMD_CATALOG,          /// 目录信息
    EMANSCDP_SUB_CMD_DEVICE_INFO,      /// 设备信息
    EMANSCDP_SUB_CMD_RECORD_INFO,      /// 录像信息
}EManscdpCmdType;

#define XML_DEVICE_ID_LEN      21
#define XML_DEVICE_NAME_LEN    64
#define XML_GENERAL_ITEM_LEN   64
#define XML_MIN_ITEM_LEN        8

#define XML_MANSCDP_VALUE_LEN   32
#define XML_MANSCDP_NOTIFY_STATUS_OK        1
#define XML_MANSCDP_NOTIFY_STATUS_NONOK     0

/// 信令保活
typedef struct tagNotifyKeepaliveMsg
{
    int     sn;
    char    device_id[XML_DEVICE_ID_LEN];
    int     status;
    char    info[XML_GENERAL_ITEM_LEN];
}TNotifyKeepaliveMsg;

/// record query response item
struct CRecordInfoItem
{
    enum ERecordType
    {
        ERECORD_TYPE_NULL,
        ERECORD_TYPE_TIME,
        ERECORD_TYPE_ALARM,
        ERECORD_TYPE_MANUAL,
        ERECORD_TYPE_ALL,
    };
    
    char        device_id[XML_DEVICE_ID_LEN];
    char        name[XML_DEVICE_NAME_LEN];
    char        file_path[XML_GENERAL_ITEM_LEN];
    char        address[XML_GENERAL_ITEM_LEN];
    long int    start_time;
    long int    end_time;
    int         secrecy;
    ERecordType type;                               /// optional
    char        recorder_id[XML_DEVICE_ID_LEN];     /// optional
    CRecordInfoItem *next;
};

/// 目录查询结果:总括
struct CManscdpResRecordInfo
{
    int     sn;
    char    device_id[XML_DEVICE_ID_LEN];
    char    name[XML_DEVICE_NAME_LEN];
    int     sum_num;
    int     dev_list_num;

    CRecordInfoItem *item;
};


/// 目录查询结果:资源条目
struct TQueryCatalogItem
{
    char    device_id[XML_DEVICE_ID_LEN];
    char    name[XML_DEVICE_NAME_LEN];

    char    manufacturer[XML_GENERAL_ITEM_LEN];
    char    model[XML_GENERAL_ITEM_LEN];
    char    owner[XML_GENERAL_ITEM_LEN];
    char    civil_code[XML_GENERAL_ITEM_LEN];
    char    address[XML_GENERAL_ITEM_LEN];
    int     parental;
    int     safe_way;
    int     register_way;
    int     secrecy;
    int     status;

    struct TQueryCatalogItem *next;
};

/// 目录查询结果:总括
typedef struct tagManscdpQueryCatalogRes
{
    int     sn;
    char    device_id[XML_DEVICE_ID_LEN];
    int     sum_num;
    int     dev_list_num;

    TQueryCatalogItem *item;
}TManscdpQueryCatalogRes;

/// MANSCDP 结果返回
struct TManscdpXmlResult
{
    EMsgManscdpYpe manscdp_type;
    EManscdpCmdType sub_cmd_type;

    void    *value;
    int     value_len;

    int sn;
    char device_id[XML_MANSCDP_VALUE_LEN];
    int status;
};

TINYXML2_API HXmlParser xml_create();

TINYXML2_API void xml_parse_text(HXmlParser handle, char *text, TManscdpXmlResult **result);

TINYXML2_API void xml_del(HXmlParser handle);

TINYXML2_API int xml_copy_manscdp_result(TManscdpXmlResult *dst_result, TManscdpXmlResult *src_result);

TINYXML2_API void xml_free_manscdp_result(TManscdpXmlResult *result);

#ifdef __cplusplus
}
#endif


#endif  // __XML_PARSER_H__
