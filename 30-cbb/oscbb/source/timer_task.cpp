#include <stdio.h>
#include <stdlib.h>

#ifdef __MSC_VER
#define EINVAL      1
#define ENOMEM      14
#else
#include <errno.h>
#endif

#include "oscbb.h"
#include "timer_task.h"


namespace CBB
{

    /*
    * API
    * ����������
    */
    int cbb_timer_task_create(HOscbbHandle *handle, uint32_t task_num)
    {
        if (NULL == handle)
        {
            return -EINVAL;
        }

        CTimerTask  *obj;

        try
        {
            obj = new CTimerTask;
        }
        catch (...)
        {
            return -ENOMEM;
        }

        int ret = obj->create(task_num);
        if (0 != ret)
        {
            return ret;
        }

        *handle = obj;
        return ret;
    }

    /*
    * ���ӵ�������
    */
    int cbb_timer_task_add(HOscbbHandle handle, uint32_t sche_period, void(*routine)(void *arg),
        void *ctx, uint32_t cnt)
    {
        CTimerTask  *obj = static_cast<CTimerTask  *>(handle);
        return obj->add(sche_period, routine, ctx, cnt);
    }

    /*
    * ִ�е�����
    */
    int cbb_timer_task_sche(HOscbbHandle handle, uint64_t cur_time)
    {
        CTimerTask  *obj = static_cast<CTimerTask  *>(handle);
        obj->schedule(cur_time);

        return 0;
    }

}   /// end namespace CBB


    /// ���캯��
CTimerTask::CTimerTask()
{
    queue_len = 0;
    queue = NULL;
}

/// ����������
int CTimerTask::create(uint32_t len)
{
    try
    {
        queue = new CNode[len];
    }
    catch (...)
    {
        return -ENOMEM;
    }

    return 0;
}

/// ���ӵ�������,ʱ�䵥λ:ms 
int CTimerTask::add(uint32_t sche_period, void(*routine)(void *arg), void *ctx, uint32_t cnt)
{
    if (NULL == routine)
    {
        return -EINVAL;
    }

    for (uint32_t i = 0; i < queue_len; i++)
    {
        if (NULL == queue[i].timer_routine)
        {
            queue[i].last_sche_time = 0;
            queue[i].sche_period = sche_period;
            queue[i].sched_count = 0;
            queue[i].max_count = cnt;
            queue[i].timer_routine = routine;
            queue[i].user_context = ctx;

            return 0;
        }
    }

    return -EAGAIN;
}

void CTimerTask::schedule(uint64_t cur_time)
{
    for (uint32_t i = 0; i < queue_len; i++)
    {
        queue[i].schedule(cur_time);
    }
}


CTimerTask::CNode::CNode()     /// ���캯��
{
    last_sche_time = 0;
    sche_period = 0xFFFFFFFF;
    sched_count = 0;
    timer_routine = NULL;
    user_context = NULL;
}

/// ʵ�ʵ�����ת
void CTimerTask::CNode::schedule(uint64_t cur_time)
{
    if (NULL == timer_routine)
    {
        return;
    }

    /// ʱ���ж�
    if (last_sche_time + sche_period < cur_time)
    {
        /// ����
        timer_routine(user_context);
        sched_count++;

        last_sche_time = cur_time;

        /// �Ƿ�ȡ�������ʸ�
        if ((max_count) && (max_count < sched_count))
        {
            last_sche_time = 0;
            sche_period = 0xFFFFFFFF;
            sched_count = 0;
            timer_routine = NULL;
            user_context = NULL;
        }
    }
}



