

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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#endif

#include "oscbb.h"

#define cbb_printf(format, ...)     printf(format, ##__VA_ARGS__)

namespace  CBB
{
/*
 * miliseconds
 */
uint64_t gettime_ms(void)
{
#ifdef _MSC_VER
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec >> 20);
#endif
}


/*
 * safe send
 */
int cbb_send(SOCKHANDLE fd, void *buf, int len, int flags, bool block)
{
    int ret;
    int size     = len;
    size_t offset   = 0;

repeat:
    ret = send(fd, (char *)buf + offset, size, flags);
    if (ret < 0)
    {
        if (EINTR == errno)
        {
            goto repeat;
        }

        printf("send falied,errno:%d\n", errno);
        return -1;
    }
    else if (ret < size)
    {
        size    = size - ret;
        offset  = len - size;
        if (block)
        {
            goto repeat;
        }
        
        return offset;
    }

    return 0;
}

/*
 * safe send
 */
int cbb_sendto(SOCKHANDLE fd, void *buf, int len, const void *addr, int addr_len, int flags)
{
    int ret;

repeat:
#ifdef _MSC_VER
    ret = sendto(fd, (const char *)buf, len, 0, (const struct sockaddr *)addr, addr_len);
#else
    ret = sendto(fd, buf, len, 0, (const struct sockaddr *)addr, (socklen_t)addr_len);
#endif
    if (ret < 0)
    {
        if (EINTR == errno)
        {
            goto repeat;
        }

        printf("send falied,errno:%d\n", errno);
        return -1;
    }

    return 0;
}


/*
 * recv data
 */
int OSCBB_API cbb_recvfrom(SOCKHANDLE fd, void *buf, int buf_len, void  *addr, int *addr_len, int flags)
{
    int ret;

again:
#ifdef _MSC_VER
    ret = recvfrom(fd, (char *)buf, buf_len, flags, (struct sockaddr *)addr, addr_len);
#else
    ret = recvfrom(fd, buf, buf_len, flags, (struct sockaddr *)addr, (socklen_t *)addr_len);
#endif
    if (ret < 0)
    {
        if (EAGAIN == errno)
        {
            goto again;
        }

        printf("socket recv data failed, errno:%d\n", errno);
        return -1;
    }
    else if (0 == ret)
    {
        printf("socket recv no data, peer maybe shutdown\n");
        return -1;
    }

    return ret;
}

/*
 * recv data
 */
int cbb_recv(int fd, void *buf, int len, int flags)
{
    int ret;

again:
#ifdef _MSC_VER
    ret = recv(fd, (char *)buf, len, flags);
#else
    ret = recv(fd, buf, len, flags);
#endif
    if (ret < 0)
    {
        if (EAGAIN == errno)
        {
            goto again;
        }

        printf("socket recv data failed, errno:%d\n", errno);
        return -1;
    }
    else if (0 == ret)
    {
        printf("socket recv no data, peer maybe shutdown\n");
        return -1;
    }

    return ret;
}

void cbb_delay(int ms)
{
#ifdef _MSC_VER
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}


typedef struct 
{
#ifdef _LINUX_
    pthread_mutex_t     m_mutex;
#endif
}TOsMutex;

/*
 * create mutex
 */
int cbb_mutex_create(HOsMutex *mutex)
{
    return 0;
}

/*
 * delete mutex
 */
int cbb_mutex_delete(HOsMutex mutex)
{
    int ret = 0;

    TOsMutex *p = (TOsMutex *)mutex;

#ifdef _LINUX_
    ret = pthread_mutex_destroy(&p->m_mutex);
    free(p);
#endif

    return ret;
}

/*
 * lock mutex
 */
int cbb_mutex_lock(HOsMutex mutex)
{
    int ret = 0;
    TOsMutex *p = (TOsMutex *)mutex;
#ifdef _LINUX_
    ret = pthread_mutex_lock(&p->m_mutex);
    if (0 != ret)
    {
        cbb_printf("mutex lock fialed,ret:%d\n", ret); 
    }
#endif

    return ret;
}

/*
 * unlock mutex
 */
int cbb_mutex_unlock(HOsMutex mutex)
{
    int ret = 0;
    TOsMutex *p = (TOsMutex *)mutex;
#ifdef _LINUX_
    ret = pthread_mutex_unlock(&p->m_mutex);
    if (0 != ret)
    {
        cbb_printf("mutex unlock fialed,ret:%d\n", ret); 
    }
#endif

    return ret;
}


/*
 * get long int from str
 */
bool cbb_str2l(char *str, long int &val)
{
    errno = 0;    /* To distinguish success/failure after call */
    char *endptr;
    val = strtol(str, &endptr, 10);

    /* Check for various possible errors */

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                || (errno != 0 && val == 0)) {
        return false;
    }

    if (endptr == str) {
        return false;
    }

    return true;

}


}   // end of namespace::CBB
