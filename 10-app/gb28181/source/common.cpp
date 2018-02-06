
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <sys/time.h>

#include "common.h"



// Automatically lock in constructor
CGuard::CGuard(pthread_mutex_t& lock):
    m_Mutex(lock),
    m_iLocked()
{
#ifndef WIN32
    m_iLocked = pthread_mutex_lock(&m_Mutex);
#else
    m_iLocked = WaitForSingleObject(m_Mutex, INFINITE);
#endif
}

// Automatically unlock in destructor
CGuard::~CGuard()
{
#ifndef WIN32
    if (0 == m_iLocked)
      pthread_mutex_unlock(&m_Mutex);
#else
    if (WAIT_FAILED != m_iLocked)
      ReleaseMutex(m_Mutex);
#endif
}

void CGuard::enterCS(pthread_mutex_t& lock)
{
#ifndef WIN32
    pthread_mutex_lock(&lock);
#else
    WaitForSingleObject(lock, INFINITE);
#endif
}

void CGuard::leaveCS(pthread_mutex_t& lock)
{
#ifndef WIN32
    pthread_mutex_unlock(&lock);
#else
    ReleaseMutex(lock);
#endif
}

void CGuard::createMutex(pthread_mutex_t& lock)
{
#ifndef WIN32
    pthread_mutex_init(&lock, NULL);
#else
    lock = CreateMutex(NULL, false, NULL);
#endif
}

void CGuard::releaseMutex(pthread_mutex_t& lock)
{
#ifndef WIN32
    pthread_mutex_destroy(&lock);
#else
    CloseHandle(lock);
#endif
}

void CGuard::createCond(pthread_cond_t& cond)
{
#ifndef WIN32
    pthread_cond_init(&cond, NULL);
#else
    cond = CreateEvent(NULL, false, false, NULL);
#endif
}

void CGuard::releaseCond(pthread_cond_t& cond)
{
#ifndef WIN32
    pthread_cond_destroy(&cond);
#else
    CloseHandle(cond);
#endif

}

const int CLog::CLOG_LEVEL_API      = 3;
const int CLog::CLOG_LEVEL_GB_TASK  = 10;     

void CLog::log(int level, const char* format, ...)
{
    va_list pv;
    va_start(pv, format);
    vprintf(format, pv);
    va_end(pv);
}

uint32_t CRandom::rand()
{
    uint32_t dwHash = 0;

    dwHash += (0x5FFFFFFF & get_random());

    return (dwHash & 0x7FFFFFFF);
}

/*
 * get random
 */
uint32_t CRandom::get_random()
{
    uint32_t ret;
    int     index = 0;
    static struct timeval tv; 

again:
    gettimeofday(&tv, NULL);

    // 引入随机数
    tv.tv_usec++;
    srand(tv.tv_usec);
    ret = random();
    index++;

    if ( (0 == ret) && (index < 3) )
    {
        goto again;
    }

    return ret;
}


/*
 * get long int from str
 */
bool CStr2Digit::get_lint(char *str, long int &val)
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

/*
 * miliseconds
 */
uint64_t CTime::gettime_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (ts.tv_nsec >> 20);
}

int CTime::gettime_localtime(struct tm *tmp)
{
    time_t now_time;
    if ( -1 == time(&now_time))
    {
        return -errno;
    }

    localtime_r(&now_time, tmp);
    return 0;
}

