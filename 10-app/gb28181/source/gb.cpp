/*
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * multiple format streaming server based on the FFmpeg libraries
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include "common.h"
#include "gb.h"
#include "gbapi.h"

#include "gb_server.h"



namespace GB
{
    /// 国标服务器参数配置项构造函数
    CGbSvrParam::CGbSvrParam() 
    {
        /// GB28181 stack server param
        memset(ip, 0, sizeof(ip));
        port                = 0;               /// 国标服务端口

        memset(sip_id, 0, sizeof(sip_id));
        memset(domain_id, 0, sizeof(domain_id));
        memset(user_agent, 0, sizeof(user_agent));

        memset(inter_addr, 0, sizeof(inter_addr));
        memset(external_addr, 0, sizeof(external_addr));
        external_port               = 0;

        memset(ncproxy_addr, 0, sizeof(ncproxy_addr));
        ncproxy_port                = 0;                /// 国标服务客户端通信端口

        max_reg_dev_num             = 1024;         /// 国标服务器最大接入设备数
        max_res_num                 = 1024;         /// 国标服务器最大资源条目数目(目录查询条目)

        reg_period                  = 3600;             /// 回话注册时间
        session_linger_time         = 60000;            /// 会话保持时间,单位:ms
        manual_start                = true;             /// 若为true,需要调用gb_server_start才能开启服务
    }

    CGbManscdpParam::CGbManscdpParam()
    {
        memset(device_id, 0, sizeof(device_id));
        memset(domain_id, 0, sizeof(domain_id));
        user_agent      = NULL;                         /// 用户代理名称
        device_name     = NULL;                         /// 必选 设备名称
        manufacturer    = NULL;                         /// 必选 设备厂商
        model           = NULL;                         /// 必选 设备型号
        owner           = NULL;                         /// 必选 设备归属
        civil_code      = NULL;                         /// 必选 行政区域
        address         = NULL;                         /// 必选 安装地址
        parental                = true;                 /// 必选 是否有子设备
        on                      = true;                 /// 必选 设备状态
    }

    CGbDev::CGbDev()        // construction function
    {
        memset(dev_id, 0, GBSIP_USERNAME_LEN);
        memset(dev_domain_id, 0, GBSIP_USERNAME_LEN);

        src_host                = 0;       // network order
        src_port                = 0;       // network order
        last_update_time        = 0;       // millisecond
    }

    CGbDev::~CGbDev()       // destruction function
    {
    }

    CGbItem::CGbItem()      // construction function
    {
        memset(device_id, 0, sizeof(device_id));
        memset(name, 0, sizeof(name));

        memset(manufacturer, 0, sizeof(manufacturer));
        memset(model, 0, sizeof(model));
        memset(owner, 0, sizeof(owner));
        memset(civil_code, 0, sizeof(civil_code));
        memset(address, 0, sizeof(address));

        parental            = 0;
        safe_way            = 0;
        register_way        = 0;
        secrecy             = 0;
        status              = 0; 
        dev                 = NULL;;
    };
    
    CGbItem::~CGbItem()      // construction function
    {
    }

    // gb init function
    int gb_init()
    {
        return CGbServer::init();
    }

    /// 创建国标服务器
    int gb_server_create(gb_handle_t *handle, CGbSvrParam &param)
    {
        CGbServer *server;
        try
        {
            server = new CGbServer;
        }
        catch (...)
        {
            return -ENOMEM;
        }

        int ret = server->create(param);
        if (0 == ret)
        {
            *handle = server;
        }
        else
        {
            delete server;
        }

        return ret;
    }


    int gb_server_start(gb_handle_t handle)
    {
        CGbServer *server = static_cast<CGbServer *>(handle);;
        return server->start(handle);
    }

    int gb_server_set_cb(gb_handle_t handle, void *context, int (*cb)(void *ctx, int type, void *var, int var_len))
    {
        return CGbApi::server_set_ev_cb(handle, context, cb);
    }


    int gb_server_invite(gb_handle_t handle, MN::CMediaInvite &invite_param)
    {
        return CGbApi::server_invite(handle, invite_param);
    }

    int gb_server_reponse_catalog(gb_handle_t handle, MN::CRmtQueryCatalog &query_catalog_param, void *catalog_buf, int catalog_buf_len)
    {
        return CGbServer::reponse_catalog(handle, query_catalog_param, catalog_buf, catalog_buf_len);
    }

    int gb_server_rmt_invite(gb_handle_t handle, MN::CMediaInvite &invite_param)
    {
        return CGbServer::rmt_invite(handle, invite_param);
    }
}
