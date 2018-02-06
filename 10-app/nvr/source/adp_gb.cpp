
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>



#include "common.h"
#include "gb.h"
#include "adp_gb.h"
#include "msgno.h"
using namespace MN;

// using namespace GB;

/// define
class CAdpGbParam
{
public:
    void set_gb_handle(gb_handle_t gb_handle);

    /// real run function
    void *run();

    /// start adpter
    int start();

    /// adapter routine
    static void *routine(void *arg);

    /// deal income msg
    void deal_msg();

    static int send_msg(void *ctx, int type, void *var, int var_len);

    int send_msg(int type, void *var, int var_len);


private:
    pthread_t           pid;
    pthread_mutex_t     mutex;
    gb_handle_t         handle;

    int                 fd;
    struct sockaddr_un  local_addr;
    struct sockaddr_un  peer_addr;
    socklen_t           addr_len;
    ssize_t             recv_len;
    char                buf[1500];
    CMsgFormat          recv_msg;

    bool                task_run;
    bool                task_exit;
};

int CAdpGbParam::send_msg(void *ctx, int type, void *var, int var_len)
{
    CAdpGbParam *param  = static_cast<CAdpGbParam *>(ctx);

    CLog::log(CLog::CLOG_LEVEL_API, "deal msg from gb28181\n");
    return param->send_msg(type, var, var_len);
}

int CAdpGbParam::send_msg(int type, void *var, int var_len)
{
    int ret;
    
    printf("gbadp send msg now\n");
    peer_addr.sun_family    = AF_UNIX;
    snprintf(peer_addr.sun_path, sizeof(peer_addr.sun_path), "%s", "/var/run/vsserver_msgcenter_0");

    ret = CBB::cbb_sendto(fd, var, var_len, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    if (ret < 0)
    {
        printf("gbadp send msg failed\n");
        return -1;
    }

    return 0;
}

/// deal income msg
void CAdpGbParam::deal_msg()
{
    int ret = 0;

    // check msg len 
    if (recv_len < (int)sizeof(recv_msg))
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gb adp,invalid msg len:%d\n", recv_len);
        return;
    }

    // check data len
    memcpy(&recv_msg, buf, sizeof(recv_msg));
    if (recv_msg.var_len + (int)sizeof(recv_msg) != recv_len)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "mc,invalid msg len:%d, var len:%d\n", recv_len, recv_msg.var_len);
        return;
    }

    recv_msg.var    = buf + sizeof(recv_msg);

    CLog::log(CLog::CLOG_LEVEL_API, "[GB]deal income msg, msg_type:%d\n", recv_msg.type);
    switch (recv_msg.type)
    {
        case MN_GB_INVITE_MEDIA:
            {
                MN::CMediaInvite invite_param;
                if (recv_msg.var_len != sizeof(invite_param))
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&invite_param, recv_msg.var, sizeof(invite_param));
                    ret = GB::gb_server_invite(handle, invite_param);
                }
            }
            break;

        case MN_GB_LOCAL_RES_CATALOG:
            {
                MN::CRmtQueryCatalog query_catalog_param;
                if ( (recv_msg.var_len < (int)sizeof(query_catalog_param))
                     || (((recv_msg.var_len - sizeof(query_catalog_param)) % sizeof(MN::CCatalogItem)) != 0) )
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&query_catalog_param, recv_msg.var, sizeof(query_catalog_param));
                    ret = GB::gb_server_reponse_catalog(handle, query_catalog_param, (uint8_t *)recv_msg.var + sizeof(CRmtQueryCatalog),
                                recv_msg.var_len - sizeof(query_catalog_param));
                }
            }
            break;

        case MN_GB_RMT_INVITE_MEDIA:
            {
                MN::CMediaInvite invite_param;
                if (recv_msg.var_len != sizeof(invite_param))
                {
                    ret = -1;
                }
                else
                {
                    memcpy(&invite_param, recv_msg.var, sizeof(invite_param));
                    ret = GB::gb_server_rmt_invite(handle, invite_param);
                }
            }
            break;

        default:
            break;
    }

    if (ret < 0)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "gb adp deal msg failed:%d\n", ret);
    }
}

/// adapter routine
void *CAdpGbParam::routine(void *arg)
{
    CAdpGbParam *param  = static_cast<CAdpGbParam *>(arg);

    return param->run();
}

/// real run function
void *CAdpGbParam::run()
{
    int ret;
    ret = GB::gb_server_start(handle);
    if (ret < 0)
    {
        printf("start gbserver failed,ret:%d\n", ret);
        return NULL;
    }

    while(true)
    {
        CGuard::enterCS(mutex);

        if (true == task_exit)
        {
            CLog::log(CLog::CLOG_LEVEL_API, "msgserver is commanded to exit\n");
            CGuard::leaveCS(mutex);
            break;
        }

        // if not start,don't recv msg
        if (false == task_run)
        {
            usleep(10000);
            CGuard::leaveCS(mutex);
            continue;
        }

        addr_len    = sizeof(peer_addr);
        recv_len = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&peer_addr, &addr_len);
        if (recv_len < 0)
        {
            CGuard::leaveCS(mutex);
            if ( (EINTR == errno) || (EAGAIN == errno) )
            {
                continue;
            }

            CLog::log(CLog::CLOG_LEVEL_API, "msgcenter recvfrom failed, task to exit, errno:%d\n", errno);
            break;
        }
        else if (0 == recv_len)
        {
            CGuard::leaveCS(mutex);
            continue;
        }

        deal_msg();
        CGuard::leaveCS(mutex);
    }


    CLog::log(CLog::CLOG_LEVEL_API, "set sock addr reuse failed,errno:%d\n", errno);
    return NULL;
}

/// start adpter
int CAdpGbParam::start()
{
    return 0;
}

void CAdpGbParam::set_gb_handle(gb_handle_t gb_handle)
{
    handle  = gb_handle;
}

/// construction function
CAdpGb::CAdpGb()
{
    param = NULL;
}

/// get ogj
CAdpGb *CAdpGb::get_instance()
{
    static CAdpGb   adp_gb;
    return &adp_gb;
}

int CAdpGb::init(CFG::CCfgParam &cfg)
{
    int ret;
    gb_handle_t handle;

    try
    {
        get_instance()->param = new CAdpGbParam;
    }
    catch(...)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "maybe no mem\n");
        ret = -ENOMEM; 
        return ret;
    }

    ret = GB::gb_init();
    if (ret != GB_SUCCESS)
    {
        printf("init gb failed,ret:%d\n", ret);
        return ret;
    }

    /// 创建国标服务器对象
    GB::CGbSvrParam param;
    snprintf(param.ip, sizeof(param.ip), "%s", cfg.gbs_ip);
    param.port      = cfg.gbs_port;
    snprintf(param.sip_id, sizeof(param.sip_id), "%s", cfg.gbs_sip_id);
    snprintf(param.domain_id, sizeof(param.domain_id), "%s", cfg.gbs_domain_id);
    snprintf(param.user_agent, sizeof(param.user_agent), "%s", cfg.gbs_user_agent);
    snprintf(param.inter_addr, sizeof(param.inter_addr), "%s", cfg.gbs_inter_addr);
    snprintf(param.external_addr, sizeof(param.external_addr), "%s", cfg.gbs_external_addr);
    param.external_port     = cfg.gbs_external_port;
    snprintf(param.ncproxy_addr, sizeof(param.ncproxy_addr), "%s", cfg.gbs_ncproxy_addr);
    param.ncproxy_port      = cfg.gbs_ncproxy_port;
    param.max_reg_dev_num   = cfg.gbs_max_reg_dev_num;
    param.max_res_num       = cfg.gbs_max_res_num;
    param.session_linger_time   = cfg.gbs_session_linger_time;
    param.manual_start          = false;
    ret = GB::gb_server_create(&handle, param);
    if (GB_SUCCESS != ret)
    {
        printf("create gbserver failed,ret:%d\n", ret);
        return ret;
    }

    ret = GB::gb_server_set_cb(handle, get_instance()->param, CAdpGbParam::send_msg);
    if (GB_SUCCESS != ret)
    {
        printf("set gb callback failed,ret:%d\n", ret);
        return ret;
    }

    get_instance()->param->set_gb_handle(handle);
    
    ret = get_instance()->param->start();
    if (0 != ret)
    {
        CLog::log(CLog::CLOG_LEVEL_API, "start adpter routine failed,ret:%d\n", ret);

        return ret;
    }

    printf("gb server start successfully\n");
    return ret;
}



/// deinit AdpGb
int CAdpGb::deinit()
{
    return 0;
}
