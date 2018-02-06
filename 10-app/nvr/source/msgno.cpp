

#include <string.h>

#include "msgno.h"


/// namespace MN start
namespace MN
{

CDevRegister::CDevRegister()
{
    memset(device_id, 0, sizeof(device_id));
    memset(name, 0, sizeof(name));
    memset(ip, 0, sizeof(ip));
    memset(port, 0, sizeof(port));
}


CCatalogItem::CCatalogItem()
{
    memset(this, 0, sizeof(*this));
}


CNetSession::CNetSession()
{
    rtp_port = rtcp_port = 0; rtp_ip = rtcp_ip = 0;
}

CMediaInvite::CMediaInvite()
{
    memset(device_id, 0, sizeof(device_id));
    dev_ip      = 0;
    dev_port    = 0;
    ssrc        = 0;
}

CRmtQueryCatalog::CRmtQueryCatalog()
{
    memset(device_id, 0, sizeof(device_id));
    rmt_ip      = 0;
    rmt_port    = 0;
    num         = 0;
    sn          = 0;
}



/*******************************************************************************/
/// new protocol
/*******************************************************************************/
void CStreamHdr::buf2hdr(uint8_t *buf, CStreamHdr &str_hdr)
{
    str_hdr.magic  = (uint16_t(buf[0]) << 8) | buf[1]; 
    str_hdr.len    = (uint16_t(buf[2]) << 8) | buf[3];
}

void CStreamHdr::hdr2buf(uint8_t *buf, CStreamHdr &str_hdr)
{
    buf[0] = 0x24;
    buf[1] = 0x40;

    buf[2] = (str_hdr.len >> 8) & 0xFF;
    buf[3] = str_hdr.len & 0xFF;
}


CMsgHdr::CMsgHdr(uint16_t _seq, uint32_t _type):ver(MC_MSG_PROTOCOL_VER01),crc(0),seq(_seq),type(_type)
{
}

CMsgHdr::CMsgHdr()
{
    ver     = 0;
    crc     = 0;
    seq     = 0;
    type    = 0;
}


void CMsgHdr::buf2hdr(uint8_t *buf, CMsgHdr &msg_hdr)
{
    msg_hdr.ver = buf[0];
    msg_hdr.crc = buf[1];
    msg_hdr.seq = ((uint16_t)buf[2] << 8 ) | buf[3];
    msg_hdr.type = ((uint32_t)buf[4] << 24) | ((uint32_t)buf[5] << 16) | ((uint32_t)buf[6] << 8) | buf[7];
}

bool CMsgHdr::crc_match(uint8_t *buf)
{
    uint8_t crc_bak = buf[1];
    buf[1]  = 0;
    uint8_t expected_crc = tb_crc8(buf, 8);
    buf[1]  = crc_bak;

    return (crc_bak == expected_crc) ? true : false;
}

uint8_t CMsgHdr::tb_crc8(uint8_t *data, int len)
{
    int i, j;
    uint8_t val = 0xff;
    for (i = 0; i < len; i++) {
        val ^= data[i];
        for (j = 0; j < 8; j++)
          val = (val << 1) ^ ((val & 0x80) ? 7 : 0);
    }
    return val;
}

/// buf must greater than 8 bytes
bool CMsgHdr::msg2buf_calc_crc(uint8_t *buf, uint32_t &buf_len, CMsgHdr &msg_hdr)
{
    if (buf_len < 8)
    {
        return false;
    }

    buf_len = 8;

    buf[0] = msg_hdr.ver;
    buf[1] = 0;
    buf[2] = (msg_hdr.seq >> 8) & 0xFF;
    buf[3] = msg_hdr.seq & 0xFF;

    buf[4] = (msg_hdr.type >> 24) & 0xFF;
    buf[5] = (msg_hdr.type >> 16) & 0xFF;
    buf[6] = (msg_hdr.type >> 8) & 0xFF;
    buf[7] = msg_hdr.type & 0xFF;

    /// calc crc
    buf[1] = tb_crc8(buf, 8);
    return true;
}

/// construntion
CMsgGbQueryRecordInfo::CMsgGbQueryRecordInfo()
{
    memset(this, 0, sizeof(*this));
}

CMsgGbResRecordInfo::CMsgGbResRecordInfo()
{
    memset(this, 0, sizeof(*this));
}

CMsgGbRecordItem::CMsgGbRecordItem()
{
    memset(this, 0, sizeof(*this));
}

#if 0
struct MN_API CMsgGbQueryRecordInfo
{
    static bool buf2msg(uint8_t *buf, uint32_t len, CMsgAddSwitchIpv4 &msg);

    static bool msg2buf(uint8_t *buf, uint32_t &buf_len, CMsgAddSwitchIpv4 &msg);

    char    device_id[MN_DEV_ID_LEN];
    char    catalog_id[MN_DEV_ID_LEN];
    int64_t start_time;
    int64_t end_time;
};
#endif



bool CMsgAddSwitchIpv4::buf2msg(uint8_t *buf, uint32_t len, CMsgAddSwitchIpv4 &msg)
{
    if (len != 24)
    {
        return false;
    }

    int index = 0;
    msg.channel_id  = ((uint32_t)buf[index] << 24) | ((uint32_t)buf[index + 1] << 16) | ((uint32_t)buf[index + 2] << 8) | buf[index + 3];

    index = 4;
    msg.to_ip       = ((uint32_t)buf[index] << 24) | ((uint32_t)buf[index + 1] << 16) | ((uint32_t)buf[index + 2] << 8) | buf[index + 3];

    index = 8;
    msg.spy_ip      = ((uint32_t)buf[index] << 24) | ((uint32_t)buf[index + 1] << 16) | ((uint32_t)buf[index + 2] << 8) | buf[index + 3];

    index = 12;
    msg.to_port     = ((uint16_t)buf[index] << 8) | buf[index + 1];

    index = 14;
    msg.spy_port    = ((uint16_t)buf[index] << 8) | buf[index + 1];

    index = 16;
    msg.ssrc        = ((uint32_t)buf[index] << 24) | ((uint32_t)buf[index + 1] << 16) | ((uint32_t)buf[index + 2] << 8) | buf[index + 3];

    index = 20;
    msg.op_result   = ((uint32_t)buf[index] << 24) | ((uint32_t)buf[index + 1] << 16) | ((uint32_t)buf[index + 2] << 8) | buf[index + 3];

    return true;
}

/// 协议解析器
CMsgParser::CMsgParser()                   /// 构造函数
{
    parser_buf      = NULL;
    parser_buf_len  = 0;
    valid_data_len  = 0;
    msg_deal_func   = NULL;
    msg_deal_ctx    = NULL;
}

CMsgParser::~CMsgParser()                  /// 析构函数
{
    if (parser_buf)
    {
        delete [] parser_buf;
        parser_buf  = NULL;
    }

    valid_data_len  = 0;
    parser_buf_len  = 0;
    msg_deal_func   = NULL;
    msg_deal_ctx    = NULL;
}

/// 创建协议解析器
int CMsgParser::create(uint32_t len)
{
    try
    {
        parser_buf  = new uint8_t [len];
    }
    catch (...)
    {
        return -1;
    }
    parser_buf_len  = len;

    return 0;
}

/// 消息处理句柄
void CMsgParser::set_msg_handler(void (*func)(void *ctx, CMsgHdr &hdr, uint8_t *msg_buf, uint32_t msg_len, uint64_t cur_time), void *ctx)
{
    msg_deal_func   = func;
    msg_deal_ctx    = ctx;
}

/// 处理内部协议
void CMsgParser::parser_income_data(uint8_t *buf, uint32_t len, uint64_t cur_time)
{
    /// fatall error
    if (len > parser_buf_len)
    {
        return;
    }

    // check msg len 
    if (len + valid_data_len <= parser_buf_len)
    {
        memcpy(parser_buf + valid_data_len, buf, len);
        valid_data_len += len;
    }
    else
    {
        uint32_t discard_len = len  + valid_data_len - parser_buf_len;
        if (discard_len < valid_data_len)
        {
            memcpy(parser_buf, parser_buf + discard_len, valid_data_len - discard_len);
            memcpy(parser_buf + discard_len, buf, len);
            valid_data_len   = parser_buf_len;
        }
        else
        {
            memcpy(parser_buf, buf, len);
            valid_data_len    = len;
        }
    }

    /// parser protocol
    while(true)
    {
        int ret = parser_protocol(cur_time);
        if (ret > 0)
        {
            int remain_len = valid_data_len - ret;

            if (0 != remain_len)
            {
                memcpy(parser_buf, parser_buf + ret, remain_len);
                valid_data_len   = remain_len;
                continue;
            }
            
            /// 剩下的未处理的数据为0
            valid_data_len = 0;
        }

        break;
    }
}

/// 解析协议
int CMsgParser::parser_protocol(uint64_t cur_time)
{
    /// valid msg at least 12 bytes
    if (valid_data_len < 12)
    {
        return 0;
    }

    /// deal msg
    for (uint32_t i = 0; i < valid_data_len - 2; i++)
    {
        if ( (parser_buf[i] == 0x24) && (parser_buf[i + 1] == 0x40) )
        {
            /// 如果头部数据不够,可能这个就是头部,删掉前面的不完整数据
            if (valid_data_len - i < 12)
            {
                return i;
            }

            /// maybe header
            uint16_t msg_len = (uint16_t(parser_buf[i + 2]) << 8) | parser_buf[i + 3];

            /// msg len must not be less than 8 bytes 
            if (msg_len < 8)
            {
                continue;
            }

            if ((msg_len + i + 4) <= valid_data_len)
            {
                bool crc_match = MN::CMsgHdr::crc_match(parser_buf + i + 4);
                
                /// crc 匹配
                if (true == crc_match)
                {
                    /// 找到一个消息
                    /// get header
                    CMsgHdr msg_hdr;
                    msg_hdr.buf2hdr(parser_buf + i + 4, msg_hdr);
                    msg_deal_func(msg_deal_ctx, msg_hdr, parser_buf + i + 12, msg_len - 8, cur_time);

                    return (msg_len + i + 4);
                }
            }
            else
            {
                return i;
            }
        }            
    }

    return 0;
}


bool CMsgAddSwitchIpv4::msg2buf(uint8_t *buf, uint32_t &buf_len, CMsgAddSwitchIpv4 &msg)
{
    if (buf_len < 24)
    {
        return false;
    }

    buf_len = 24;

    uint32_t index = 0;
    buf[index]      = (msg.channel_id >> 24) & 0xFF;
    buf[index + 1]  = (msg.channel_id >> 16) & 0xFF;
    buf[index + 2]  = (msg.channel_id >> 8) & 0xFF;
    buf[index + 3]  = msg.channel_id & 0xFF;

    index = 4;
    buf[index]      = (msg.to_ip >> 24) & 0xFF;
    buf[index + 1]  = (msg.to_ip >> 16) & 0xFF;
    buf[index + 2]  = (msg.to_ip >> 8) & 0xFF;
    buf[index + 3]  = msg.to_ip & 0xFF;

    index = 8;
    buf[index]      = (msg.spy_ip >> 24) & 0xFF;
    buf[index + 1]  = (msg.spy_ip >> 16) & 0xFF;
    buf[index + 2]  = (msg.spy_ip >> 8) & 0xFF;
    buf[index + 3]  = msg.spy_ip & 0xFF;

    index = 12;
    buf[index]  = (msg.to_port >> 8) & 0xFF;
    buf[index + 1]  = msg.to_port & 0xFF;

    index = 14;
    buf[index]  = (msg.spy_port >> 8) & 0xFF;
    buf[index + 1]  = msg.spy_port & 0xFF;

    index = 16;
    buf[index]      = (msg.ssrc >> 24) & 0xFF;
    buf[index + 1]  = (msg.ssrc >> 16) & 0xFF;
    buf[index + 2]  = (msg.ssrc >> 8) & 0xFF;
    buf[index + 3]  = msg.ssrc & 0xFF;

    index = 20;
    buf[index]      = (msg.op_result >> 24) & 0xFF;
    buf[index + 1]  = (msg.op_result >> 16) & 0xFF;
    buf[index + 2]  = (msg.op_result >> 8) & 0xFF;
    buf[index + 3]  = msg.op_result & 0xFF;

    return true;

}
}

