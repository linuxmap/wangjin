#ifndef __MEDIA_CBB_H__     ///< 头文件宏,防止重包含
#define __MEDIA_CBB_H__     ///< 头文件宏,防止重包含


#define MEDIA_API __attribute__ ((visibility("default")))

namespace MEDIA
{

enum MEDIA_API MediaType {
    MEDIA_TYPE_UNKNOWN = -1,  ///< Usually treated as AVMEDIA_TYPE_DATA
    MEDIA_TYPE_VIDEO,
    MEDIA_TYPE_AUDIO,
    MEDIA_TYPE_DATA,          ///< Opaque data information usually continuous
    MEDIA_TYPE_SUBTITLE,
    MEDIA_TYPE_ATTACHMENT,    ///< Opaque data information usually sparse
    MEDIA_TYPE_NB
};

enum MEDIA_API RTSPTransport{
    RTSP_TRANSPORT_RTP, /**< Standards-compliant RTP */
    RTSP_TRANSPORT_RDT, /**< Realmedia Data Transport */
    RTSP_TRANSPORT_RAW, /**< Raw data (over UDP) */
    RTSP_TRANSPORT_NB
};

}
#endif  // __MEDIA_CBB_H__
