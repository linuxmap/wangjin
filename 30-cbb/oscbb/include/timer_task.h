#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__


#include <stdint.h>

#define OSCPP_TIMER_TASK_NUM        16

/// 定时任务,所有时间相关参数单位是毫秒
class CTimerTask
{
public:
    CTimerTask();

    /// 增加调度任务,时间单位:ms
    int create(uint32_t len = OSCPP_TIMER_TASK_NUM);

    /// 增加调度任务,时间单位:ms
    int add(uint32_t sche_period, void(*routine)(void *arg), void *ctx = NULL, uint32_t cnt = 0);

    /// 执行调度
    void schedule(uint64_t cur_time);

private:
    struct CNode
    {
        CNode();        /// 构造函数

                        /// 实际调度轮转
        void schedule(uint64_t cur_time);

        uint64_t    last_sche_time;                     /// 上次调度时间点
        uint32_t    sche_period;                        /// 调度周期
        uint64_t    sched_count;                        /// 调度次数
        uint64_t    max_count;                          /// 最大调度次数,为0则一直调度下去
        void(*timer_routine)(void *context);    /// 定时器执行入口
        void        *user_context;                      /// 用户参数
    } *queue;

    uint32_t queue_len;
};





#endif  // __TIMER_TASK_H__
