
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _LINUX_
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#endif

#ifdef _MSC_VER
#include <time.h>
#endif

#include "oscbb.h"

#define cbb_printf(format, ...)     printf(format, ##__VA_ARGS__)
namespace CBB
{

int cbb_task_create(TASKHANDLE &task_handle, TASKROUTE task_entry, void *param, TASKID *task_id)
{
#ifdef _MSC_VER
    task_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)task_entry, param, 0, task_id);
    if (task_handle != NULL)
    {
        return 0;
    }
#endif

#ifdef _LINUX_
    int ret = pthread_create(&task_handle, NULL, task_entry, param);
    if (0 == ret)
    {
        return 0;
    }
#endif

    return -1;
}

/*
 * create mutex 
 * linux:pthread_mutex_t, Windows:HANDLE
 */
int cbb_semb_create(SEMHANDLE &phSema)
{
#ifdef _LINUX_
    int ret = pthread_mutex_init(&phSema, NULL);
#endif

#ifdef _MSC_VER
    int ret = 0;
#endif

    return -ret;
}


/*
 * lock 
 * linux:pthread_mutex_t, Windows:HANDLE
 */
int cbb_sem_take(SEMHANDLE &hSema)
{
#ifdef _LINUX_
    int ret = pthread_mutex_lock(&hSema);
#endif

#ifdef _MSC_VER
    int ret = 0;
#endif
    return -ret;
}

/*
 * unlock 
 * linux:pthread_mutex_t, Windows:HANDLE
 */
int cbb_sem_give(SEMHANDLE &hSema)
{
#ifdef _LINUX_
    int ret = pthread_mutex_unlock(&hSema);
#endif

#ifdef _MSC_VER
    int ret = 0;
#endif
    return -ret;
}

/*
 * delay
 * linux:usleep, Windows:Sleep
 */
void cbb_task_delay(uint32_t ms)
{
#ifdef _LINUX_
    usleep(ms * 1000);
#endif

#ifdef _MSC_VER
    Sleep(ms);
#endif
}

uint32_t cbb_clk_rate_get()
{
#ifdef _LINUX_
    long slRet = sysconf(_SC_CLK_TCK);
    return (uint32_t)slRet;
#endif

#ifdef _MSC_VER
    return 0;
#endif
}

/*
 * returns the number of clock ticks
 */
clock_t _tick_get()
{
#ifdef _LINUX_
    struct tms tTime;
    clock_t tRet = times(&tTime);
    //times always valid!
    if (-1 == tRet)
    {
        tRet = (clock_t)-1 - errno;
    }
    return tRet;

#endif

#ifdef _MSC_VER
    return GetTickCount();
#endif
}


/*
 * returns the number of clock ticks
 */
uint32_t cbb_tick_get()
{
    return (uint32_t)_tick_get();
}


}   /// namespace CBB

