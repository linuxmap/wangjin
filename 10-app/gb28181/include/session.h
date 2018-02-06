

#ifndef __SESSION_H__
#define __SESSION_H__

// exosip/osip include
#include <osip2/osip.h>
#include <map>
#include <string>

class CGbServer;

class CGbRegSession
{
public:
    /// 构造函数
    CGbRegSession();

    /// 析构函数
    ~CGbRegSession();

    /// 最大注册设备数量
    int create(uint32_t num = 1024);

    /// 增加注册
    bool add_auth_reg(osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time, uint32_t reg_period = 3600);

    void schedule(CGbServer *server, uint64_t time);

private:

    class CRegNode
    {
    public:
        /// 构造函数
        CRegNode();

        /// 析构函数
        ~CRegNode();

        /// 释放资源
        void free_resource();

        std::string             device_id;      /// 设备ID

        osip_message_t          *last_reg_msg;  /// 最近的一次注册消息
        uint64_t                last_reg_time;  /// 最近的一次注册时间

        uint64_t                alive_time;     /// 保活时间
        bool                    authenticated;  /// 是否已认证
        bool                    get_catalog;    /// 是否已获取到目录结构

        uint32_t                reg_ip;         /// 对端注册IP,网络序
        uint16_t                reg_port;       /// 对端注册端口,网络序
    };

    uint32_t                    max_reg_dev_num;    /// 最大注册设备数量
    uint32_t                    cur_reg_dev_num;    /// 当前注册设备数量
    std::map<std::string, CRegNode*> dev_reg_queue;

    /// 增加元素
    bool _add(std::string &device_id, osip_message_t *msg, uint32_t ip, uint16_t port, uint64_t time);

    bool check_timeout(CRegNode *node, uint64_t time);
};


#endif
