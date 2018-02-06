

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini.h"
#include "cfg.h"
#include "oscbb.h"


namespace CFG
{
/// dec
int cfg_param_deal(void* user, const char* section, const char* name, const char* value);

/// 读取配置文件
int cfg_init(char *cfg_file, CCfgParam &param)
{
    return ini_parse(cfg_file, cfg_param_deal, &param);
}


/// 加载默认配置参数
void cfg_load_dft(CCfgParam  &)
{
}

/// 构造函数
CCfgParam::CCfgParam()
{
#if 0
    dbg_port                = 0;                   /// 调试端口
    dbg_max_reg_func_num    = 64;       /// 调试模块支持的最大命令数

    snprintf(gbs_ip, sizeof(gbs_ip), "%s", "10.10.10.101");
    gbs_port    = 5061;

    snprintf(gbs_sip_id, sizeof(gbs_sip_id), "%s", "42010000002000000012");
    snprintf(gbs_domain_id, sizeof(gbs_domain_id), "%s", "42010000");
    snprintf(gbs_user_agent, sizeof(gbs_user_agent), "%s", "IMOS/netmarch v1.1.0");

    snprintf(gbs_inter_addr, sizeof(gbs_inter_addr), "%s", "/var/run/adp_gb_0");

    /// 默认不开启国标服务器外部通信地址
    snprintf(gbs_external_addr, sizeof(gbs_external_addr), "%s", gbs_ip);
    gbs_external_port   = 0;

    snprintf(gbs_ncproxy_addr, sizeof(gbs_ncproxy_addr), "%s", gbs_ip);
    gbs_ncproxy_port   = 5070;

    gbs_max_reg_dev_num = 1024;
    gbs_max_res_num     = 1024;
#endif
}

void CCfgParam::show()
{
    /// show cfg param
    printf("---------------cfg param list start---------------\n");
    printf("[CFG]dbg port:%u,max reg func num:%u\n", dbg_port, dbg_max_reg_func_num);
    printf("[CFG]dev max reg dev num:%u,max catalog num:%u\n", dev_max_dev_num, dev_max_item_num);
    printf("[CFG]mc unix dgram addr:%s,unix stream addr:%s\n", mc_unix_dgram_addr, mc_unix_stream_addr);
    printf("[CFG]mc ipv4 stream addr:%s,ipv4 stream port:%u\n", mc_ipv4_stream_addr, mc_ipv4_stream_port);

    printf("[CFG]vtdu media recv addr:%s\n", vtdu_media_recv_ip);

    printf("[CFG]gb server addr:%s,port:%u\n", gbs_ip, gbs_port);
    printf("[CFG]gb server sip id:%s,domain:%s,agent:%s\n", gbs_sip_id, gbs_domain_id, gbs_user_agent);
    printf("[CFG]gb server inter addr:%s,external adrr:%s,external port:%u\n",
                gbs_inter_addr, gbs_external_addr, gbs_external_port);
    printf("[CFG]gb server ncproxy addr:%s,ncproxy port:%u\n", gbs_ncproxy_addr, gbs_ncproxy_port);
    printf("[CFG]gb server max reg dev num:%u,max res num:%u\n", gbs_max_reg_dev_num, gbs_max_res_num);
    printf("[CFG]gb server session linger time:%u\n", gbs_session_linger_time);
    printf("---------------cfg param list end---------------\n");
}

/// 解析媒体分发相关参数
int cfg_parser_vtdu_param(void* user, const char* name, const char* value)
{
    int ret = 1;
    CCfgParam *param = static_cast<CCfgParam *>(user);

    if (!strcmp(name, "vtdu_media_recv_ip"))
    {
        snprintf(param->vtdu_media_recv_ip, sizeof(param->vtdu_media_recv_ip), "%s", value);
    }

    return ret;
}

/// 解析设备相关参数
int cfg_parser_dev_param(void* user, const char* name, const char* value)
{
    int ret = 1;
    long int l_tmp;
    CCfgParam *param = static_cast<CCfgParam *>(user);

    if (!strcmp(name, "dev_max_item_num"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->dev_max_item_num = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }
    if (!strcmp(name, "dev_max_dev_num"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->dev_max_dev_num  = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}

/// 解析国标协议协议栈参数
int cfg_parser_gb_param(void* user, const char* name, const char* value)
{
    int ret = 1;
    long int l_tmp;
    CCfgParam *param = static_cast<CCfgParam *>(user);

    if (!strcmp(name, "server_sip_ip"))
    {
        snprintf(param->gbs_ip, sizeof(param->gbs_ip), "%s", value);
    }
    if (!strcmp(name, "server_sip_port"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->gbs_port     = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }
    else if (!strcmp(name, "server_sip_id"))
    {
        snprintf(param->gbs_sip_id, sizeof(param->gbs_sip_id), "%s", value);
    }
    else if (!strcmp(name, "server_sip_domain_id"))
    {
        snprintf(param->gbs_domain_id, sizeof(param->gbs_domain_id), "%s", value);
    }
    else if (!strcmp(name, "server_user_agent"))
    {
        snprintf(param->gbs_user_agent, sizeof(param->gbs_user_agent), "%s", value);
    }
    else if (!strcmp(name, "server_inter_addr"))
    {
        snprintf(param->gbs_inter_addr, sizeof(param->gbs_inter_addr), "%s", value);
    }
    else if (!strcmp(name, "server_external_addr"))
    {
        snprintf(param->gbs_external_addr, sizeof(param->gbs_external_addr), "%s", value);
    }
    else if (!strcmp(name, "server_external_port"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->gbs_external_port    = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }
    else if (!strcmp(name, "server_ncproxy_addr"))
    {
        snprintf(param->gbs_ncproxy_addr, sizeof(param->gbs_ncproxy_addr), "%s", value);
    }
    else if (!strcmp(name, "server_ncproxy_port"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->gbs_ncproxy_port    = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }
    else if (!strcmp(name, "server_max_reg_dev_num"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->gbs_max_reg_dev_num = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }
    else if (!strcmp(name, "server_max_res_num"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->gbs_max_res_num  = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }
    else if (!strcmp(name, "server_session_linger_time"))
    {
        if (true == CBB::cbb_str2l((char *)value, l_tmp))
        {
            param->gbs_session_linger_time  = l_tmp;
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        printf("unsupported GB cfg item:%s\n", name);
    }

    return ret;
}


/// ini解析回调处理函数,此处需要注意返回值非0为成功
int cfg_param_deal(void* user, const char* section, const char* name,
            const char* value)
{
    CCfgParam *param = static_cast<CCfgParam *>(user);
    long int l_tmp;
    int ret = 1;

    if (!strcmp(section, "GB28181"))
    {
        cfg_parser_gb_param(user, name, value);
    }
    else if (!strcmp(section, "DEBUG"))
    {
        if (!strcmp(name, "dbg_port"))
        {
            if (true == CBB::cbb_str2l((char *)value, l_tmp))
            {
                param->dbg_port = l_tmp;
            }
            else
            {
                ret = 0;
            }
        }
        else if (!strcmp(name, "dbg_max_reg_func_num"))
        {
            if (true == CBB::cbb_str2l((char *)value, l_tmp))
            {
                param->dbg_max_reg_func_num = l_tmp;
            }
            else
            {
                ret = 0;
            }
        }
        else
        {
            printf("unsupported DEBUG cfg item:%s\n", name);
        }
    }
    else if (!strcmp(section, "MC"))
    {
        if (!strcmp(name, "mc_unix_dgram_addr"))
        {
            snprintf(param->mc_unix_dgram_addr, sizeof(param->mc_unix_dgram_addr), "%s", value);
        }
        else if (!strcmp(name, "mc_unix_stream_addr"))
        {
            snprintf(param->mc_unix_stream_addr, sizeof(param->mc_unix_stream_addr), "%s", value);
        }
        if (!strcmp(name, "mc_ipv4_stream_addr"))
        {
            snprintf(param->mc_ipv4_stream_addr, sizeof(param->mc_ipv4_stream_addr), "%s", value);
        }
        else if (!strcmp(name, "mc_ipv4_stream_port"))
        {
            if (true == CBB::cbb_str2l((char *)value, l_tmp))
            {
                param->mc_ipv4_stream_port  = l_tmp;
            }
            else
            {
                ret = 0;
            }
        }
    }
    else if (!strcmp(section, "DEV"))
    {
        ret = cfg_parser_dev_param(user, name, value);
    }
    else if (!strcmp(section, "VTDU"))
    {
        ret = cfg_parser_vtdu_param(user, name, value);
    }

    return ret;
}



}   /// end namespace CFG
