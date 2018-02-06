#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include "oscbb.h"
#include "media_cbb.h"
#include "sdp.h"


#define SPACE_CHARS " \t\r\n"

int av_strstart(const char *str, const char *pfx, const char **ptr)
{
    while (*pfx && *pfx == *str) {
        pfx++;
        str++;
    }
    if (!*pfx && ptr)
      *ptr = str;
    return !*pfx;
}
/**
 *  * Locale-independent conversion of ASCII isspace.
 *   */
int cbb_isspace(int c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' ||
        c == '\t' || c == '\v';
}

void get_word_until_chars(char *buf, int buf_size,
            const char *sep, const char **pp)
{
    const char *p;
    char *q;

    p = *pp;
    p += strspn(p, SPACE_CHARS);
    q = buf;
    while (!strchr(sep, *p) && *p != '\0') {
        if ((q - buf) < buf_size - 1)
          *q++ = *p;
        p++;
    }
    if (buf_size > 0)
      *q = '\0';
    *pp = p;
}

void get_word(char *buf, int buf_size, const char **pp)
{
    get_word_until_chars(buf, buf_size, SPACE_CHARS, pp);

}

void get_word_sep(char *buf, int buf_size, const char *sep,
            const char **pp)
{
    if (**pp == '/') (*pp)++;
    get_word_until_chars(buf, buf_size, sep, pp);
}

int get_sockaddr(const char *buf, uint32_t *addr)
{
    struct addrinfo hints = { 0 }, *ai = NULL;
    int ret;

    hints.ai_flags = AI_NUMERICHOST;
    if ((ret = getaddrinfo(buf, NULL, &hints, &ai))) {
        return -1;
    }
    *addr = ((struct sockaddr_in *)(ai->ai_addr))->sin_addr.s_addr;
    freeaddrinfo(ai);
    return 0;
}
#define RTP_PT_PRIVATE 96

namespace SDP
{

/* parse the rtpmap description: <codec_name>/<clock_rate>[/<other params>] */
int sdp_parse_rtpmap(uint8_t payload_type, const char *p)
{
    char buf[256];
    int i;
    const char *c_name;

    /* See if we can handle this kind of payload.
     *      * The space should normally not be there but some Real streams or
     *           * particular servers ("RealServer Version 6.1.3.970", see issue 1658)
     *                * have a trailing space. */
    get_word_sep(buf, sizeof(buf), "/ ", &p);
    if (payload_type < RTP_PT_PRIVATE) {
        /* We are in a standard case
         *          * (from http://www.iana.org/assignments/rtp-parameters). */
        // par->codec_id = ff_rtp_codec_id(buf, par->codec_type);
    }


    get_word_sep(buf, sizeof(buf), "/", &p);
    i = atoi(buf);

    return i;
}

void sdp_parse_line(int letter, const char *buf, TSdpInfo *sdp_info, TSdpMediaDesc *media_desc)
{
    char buf1[64], st_type[64];
    const char *p;

    p = buf;
    switch (letter) {
        case 'c':
            get_word(buf1, sizeof(buf1), &p);
            if (strcmp(buf1, "IN") != 0)
              return;
            get_word(buf1, sizeof(buf1), &p);
            if (strcmp(buf1, "IP4") && strcmp(buf1, "IP6"))
              return;
            get_word_sep(buf1, sizeof(buf1), "/", &p);

            if (get_sockaddr(buf1, &sdp_info->c_ip))
              return;

            if (*p == '/') {
                p++;
                get_word_sep(buf1, sizeof(buf1), "/", &p);
                sdp_info->ttl = atoi(buf1);
            }
            break;

        case 's':
            break;

        case 'i':
            break;

        case 'm':
            {
                if (NULL == media_desc)
                  return;

                // get media type
                media_desc->media_type = MEDIA::MEDIA_TYPE_UNKNOWN;
                get_word(st_type, sizeof(st_type), &p);
                if (!strcmp(st_type, "audio")) {
                    media_desc->media_type = MEDIA::MEDIA_TYPE_AUDIO;
                } else if (!strcmp(st_type, "video")) {
                    media_desc->media_type = MEDIA::MEDIA_TYPE_VIDEO;
                } else if (!strcmp(st_type, "application")) {
                    media_desc->media_type = MEDIA::MEDIA_TYPE_DATA;
                } else if (!strcmp(st_type, "text")) {
                    media_desc->media_type = MEDIA::MEDIA_TYPE_SUBTITLE;
                }

                if (media_desc->media_type == MEDIA::MEDIA_TYPE_UNKNOWN) {
                    return;
                }

                get_word(buf1, sizeof(buf1), &p); /* port */
                media_desc->port = atoi(buf1);

                get_word(buf1, sizeof(buf1), &p); /* protocol */
                if (!strncasecmp(buf1, "udp", strlen("udp")))
                  media_desc->transport = MEDIA::RTSP_TRANSPORT_RAW;
                else if (!strncasecmp(buf1, "rtp", strlen("rtp")))
                  media_desc->transport = MEDIA::RTSP_TRANSPORT_RTP;

                /* XXX: handle list of formats */
                get_word(buf1, sizeof(buf1), &p); /* format list */
                media_desc->p_type = atoi(buf1);
            }
            break;

        case 'a':
            {
                if (av_strstart(p, "control:", &p))
                {
                    strncpy(media_desc->control_url, p, sizeof(media_desc->control_url));
                }
                else if (av_strstart(p, "rtpmap:", &p)) {
                    /* NOTE: rtpmap is only supported AFTER the 'm=' tag */
                    get_word(buf1, sizeof(buf1), &p);
                    media_desc->p_type = atoi(buf1);
                    sdp_parse_rtpmap(media_desc->p_type, p);

                    //    parse_fmtp(s, rt, payload_type, s1->delayed_fmtp);
                }
                else if (av_strstart(p, "fmtp:", &p) ||
                            av_strstart(p, "framesize:", &p)) {
                    // let dynamic protocol handlers have a stab at the line.
                    get_word(buf1, sizeof(buf1), &p);
                    media_desc->p_type = atoi(buf1);
                } else if (av_strstart(p, "ssrc:", &p)) {
                    get_word(buf1, sizeof(buf1), &p);
                    media_desc->ssrc = strtoll(buf1, NULL, 10);
                } else if (av_strstart(p, "range:", &p)) {
                } else if (av_strstart(p, "lang:", &p)) {
                } else if (av_strstart(p, "IsRealDataType:integer;",&p)) {
                } else if (av_strstart(p, "SampleRate:integer;", &p)) {
                } else if (av_strstart(p, "crypto:", &p)) {
                    // RFC 4568
                } else if (av_strstart(p, "source-filter:", &p)) {
                } else {
                }
            }
            break;
            
        case 'y':
            {
                get_word(buf1, sizeof(buf1), &p);
                media_desc->ssrc = atoi(buf1);
            }
            break;
    }
}


int sdp_parse(const char *content, int len, TSdpInfo *sdp_info, TSdpMediaDesc *media_desc, int media_desc_num)
{
    const char *p;
    int letter, i, ret = 0;
    const char *buf_end = content + len;

    char buf[256], *q;

    p = content;
    for (;;) {
        p += strspn(p, SPACE_CHARS);
        if (p >= buf_end)
        {
            return ret;
        }
        letter = *p;
        if (letter == '\0')
          break;
        p++;
        if (*p != '=')
          goto next_line;
        p++;
        /* get the content */
        q = buf;
        while (*p != '\n' && *p != '\r' && *p != '\0') {
            if ((q - buf) < (unsigned)sizeof(buf) - 1)
              *q++ = *p;
            p++;
        }
        *q = '\0';
        sdp_parse_line(letter, buf, sdp_info, media_desc);
next_line:
        if (p >= buf_end)
        {
            return ret;
        }
        while (*p != '\n' && *p != '\0')
          p++;
        if (*p == '\n')
          p++;
    }

    return ret;
}

tagSdpMediaDesc::tagSdpMediaDesc()
{
    port            = 0;
    p_type          = 255;
    media_type      = MEDIA::MEDIA_TYPE_NB;
    transport       = MEDIA::RTSP_TRANSPORT_NB;
    ssrc            = 0;
    memset(control_url, 0, sizeof(control_url));
}

tagSdpInfo::tagSdpInfo()
{
    c_ip            = 0;
    ttl             = 16;
    start_time      = 0xFFFFFFFFFFFFFFFF;
    end_time        = 0xFFFFFFFFFFFFFFFF;
}

}
