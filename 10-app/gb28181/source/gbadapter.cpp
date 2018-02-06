#include <pthread.h>
#include <stdint.h> 
#include <errno.h>

#include "common.h"
#include "gbadapter.h"



class CGbAdpUdp
{
    // eXosip_t    *ctx;
    uint16_t    port;
    uint32_t    adddr;
};


int CGbAdp::init(bool udp)
{
    int ret = 0;

    try
    {
        if (true == udp)
        {
            udp_adp = new CGbAdpUdp;
        }
    }
    catch (...)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "maybe no mem\n");
        delete udp_adp;
        ret = -ENOMEM; 
    }

    return ret;
}

