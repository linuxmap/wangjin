#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__


#include <stdint.h>

#define OSCPP_TIMER_TASK_NUM        16

/// ��ʱ����,����ʱ����ز�����λ�Ǻ���
class CTimerTask
{
public:
    CTimerTask();

    /// ���ӵ�������,ʱ�䵥λ:ms
    int create(uint32_t len = OSCPP_TIMER_TASK_NUM);

    /// ���ӵ�������,ʱ�䵥λ:ms
    int add(uint32_t sche_period, void(*routine)(void *arg), void *ctx = NULL, uint32_t cnt = 0);

    /// ִ�е���
    void schedule(uint64_t cur_time);

private:
    struct CNode
    {
        CNode();        /// ���캯��

                        /// ʵ�ʵ�����ת
        void schedule(uint64_t cur_time);

        uint64_t    last_sche_time;                     /// �ϴε���ʱ���
        uint32_t    sche_period;                        /// ��������
        uint64_t    sched_count;                        /// ���ȴ���
        uint64_t    max_count;                          /// �����ȴ���,Ϊ0��һֱ������ȥ
        void(*timer_routine)(void *context);    /// ��ʱ��ִ�����
        void        *user_context;                      /// �û�����
    } *queue;

    uint32_t queue_len;
};





#endif  // __TIMER_TASK_H__
