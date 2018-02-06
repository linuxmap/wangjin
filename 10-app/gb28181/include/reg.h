#ifndef __REG_H__
#define __REG_H__

// exosip/osip include
#include <osip2/osip.h>
#include <stdint.h>

#define GBSIP_REG_UNAUTH_NUM    64

class CGbUnauthReg
{
public:
    CGbUnauthReg();
    ~CGbUnauthReg();

    /// 创建,参数为队列长度
    int create(uint32_t len = GBSIP_REG_UNAUTH_NUM);

    /// 增加消息到注册队列,timeout为超时时间,单位:ms
    int add(osip_message_t *msg, uint64_t cur_time = 0, uint32_t timeout = 60000);

    /// 检查是否匹配，若存在并匹配返回成功
    static bool match(osip_message_t *msg);

    /// 调度器
    void update(uint64_t cur_time);

    /// 检查认证结果,true表示认证成功
    static bool check_alg(osip_message_t *msg);

private:

    /// 检查是否存在该消息登记
    /// 匹配成功后的index值表示实际队列数据位置,失败index无意义
    bool check_exist(osip_message_t *msg, uint32_t &index);


    /// 节点对象,包含注册消息、时间点、超时时间
    struct CNode
    {
        CNode();
        ~CNode();

        osip_message_t *msg;
        uint64_t        birth_time;
        uint32_t        timeout;
    } *queue;
    uint32_t    queue_len;
};



#endif  /// __REG_H__
