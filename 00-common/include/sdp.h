#ifndef __SDP_H__     ///< 头文件宏,防止重包含
#define __SDP_H__     ///< 头文件宏,防止重包含

#include <stdint.h>

#include "media_cbb.h"

#define SDP_API __attribute__ ((visibility("default")))

namespace SDP
{

typedef struct SDP_API tagSdpMediaDesc
{
    tagSdpMediaDesc();
    MEDIA::MediaType        media_type;
    MEDIA::RTSPTransport    transport;

    uint8_t                 p_type;
    uint16_t                port;
    char                    control_url[128];   /**< url for this stream (from SDP) */
    uint32_t                ssrc;

}TSdpMediaDesc;

typedef struct SDP_API tagSdpInfo
{
    tagSdpInfo();
    uint32_t        c_ip;
    int             ttl;
    uint64_t        start_time;
    uint64_t        end_time;
}TSdpInfo;

/// parse text
int SDP_API sdp_parse(const char *content, int len, TSdpInfo *sdp_info, TSdpMediaDesc *media_desc, int media_desc_num);

}

#endif  // __SDP_H__
