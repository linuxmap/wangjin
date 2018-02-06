
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include "debug.h"
#include "dev.h"
#include "mc.h"
#include "adp_gb.h"
#include "vtdu.h"
#include "nvs_core.h"


int module_init_gb28181();

/// APP data
class CNvsApp
{
public:
    /// init app
    int init(char *cfg_path);


private:
    
    CFG::CCfgParam          nvs_cfg_param;      /// NVS配置参数
    MC::CMcCfgParam         mc_cfg_param;       /// msg center configure parameter
    VTDU::CVtduCfgParam     vtdu_cfg_param;     /// vtdu configure parameter
    DEV::CDevCfgParam       dev_cfg_param;      /// DEV configure parameter

    int             vtdu_session_fd[2];


private:
    /// msg center init
    int adp_mc_init(CFG::CCfgParam &cfg);

    int module_init();

    /// 准备nvs运行数据
    int data_prepare();
}nvs_app;


/// 准备nvs运行数据
int CNvsApp::data_prepare()
{
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, vtdu_session_fd); 
    if (ret < 0)
    {
        printf("nvs prepare data failed,errno:%d\n", errno);
        return -1;
    }

    return 0;
}

/// msg center init
int CNvsApp::adp_mc_init(CFG::CCfgParam &cfg)
{
    int ret = MC::mc_init();
    if (ret < 0)
    {
        printf("msg center init failed\n");
        return -1;
    }

    snprintf(mc_cfg_param.unix_dgram_addr, sizeof(mc_cfg_param.unix_dgram_addr), "%s", cfg.mc_unix_dgram_addr);
    snprintf(mc_cfg_param.unix_stream_addr, sizeof(mc_cfg_param.unix_stream_addr), "%s", cfg.mc_unix_stream_addr);
    snprintf(mc_cfg_param.ipv4_stream_addr, sizeof(mc_cfg_param.ipv4_stream_addr), "%s", cfg.mc_ipv4_stream_addr);
    mc_cfg_param.ipv4_stream_port   = cfg.mc_ipv4_stream_port;
    mc_cfg_param.channel_vtdu_fd    = vtdu_session_fd[0];
    ret = MC::mc_create(mc_cfg_param);
    if (ret < 0)
    {
        printf("msg center create failed\n");
        return -1;
    }

    ret = MC::mc_set_cb(NULL, CNvsCore::deal_msg);
    if (ret < 0)
    {
        printf("msg center set callback failed\n");
        return -1;
    }

    ret = MC::mc_start();
    if (ret < 0)
    {
        printf("msg center start failed:%d\n", ret);
        return -1;
    }

    return ret;
}


// init all module
int CNvsApp::module_init()
{
    int ret;

    /// dbg module init
    DEBUG::TDdbInitParam dbg_param;
    dbg_param.port              = nvs_cfg_param.dbg_port;
    dbg_param.max_reg_func_num  = nvs_cfg_param.dbg_max_reg_func_num;
    ret = DEBUG::debug_init(&dbg_param);
    if (ret < 0)
    {
        printf("debug init failed\n");
    }

    vtdu_cfg_param.channel_num      = 128; // nvs_cfg_param.gbs_max_res_num;
    vtdu_cfg_param.inter_fd         = vtdu_session_fd[1];
    ret = VTDU::vtdu_init(vtdu_cfg_param);
    if (ret < 0)
    {
        printf("vtdu init failed\n");
        return -1;
    }

    dev_cfg_param.max_item_num      = 128;
    dev_cfg_param.max_dev_num       = 128;
    dev_cfg_param.media_recv_addr   = inet_addr(nvs_cfg_param.vtdu_media_recv_ip);;
    ret = DEV::dev_init(dev_cfg_param);
    if (ret < 0)
    {
        printf("dev mgr init failed\n");
        return -1;
    }

    ret = adp_mc_init(nvs_cfg_param);
    if (ret < 0)
    {
        printf("msg center init failed\n");
        return -1;
    }

    ret = CAdpGb::init(nvs_cfg_param);
    if (ret < 0)
    {
        printf("gb28181 init failed\n");
        return -1;
    }

    return 0;
}

/// init app
int CNvsApp::init(char *cfg_path)
{
    int ret = CFG::cfg_init(cfg_path, nvs_cfg_param);
    if (ret < 0)
    {
        /// use default cfg
        CFG::cfg_load_dft(nvs_cfg_param);
        printf("cfg file load failed, now get default cfg\n");
        return -1;
    }
    nvs_cfg_param.show();

    ret = data_prepare();
    if (ret < 0)
    {
        printf("prepare data failed, now to exit\n");
        return -1;
    }


    ret = module_init();
    if (ret < 0)
    {
        printf("module init failed, now to exit\n");
        return -1;
    }

    return 0;
}



/// NVS配置文件路径
char nvs_cfg_file_path[] = "/NVS/etc/config.ini";

int main(int argc, char **argv)
{
    int ret;

    ret = nvs_app.init(nvs_cfg_file_path);
    if (ret < 0) 
    {
        printf("nvs prepare failed\n");
        return -1;
    }

    for(;;) sleep(1);


    return 0;
}
