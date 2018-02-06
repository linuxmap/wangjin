#ifndef __COMMON_H__
#define __COMMON_H__
#include <sys/socket.h>
#include <stdint.h>

#include "oscbb.h"

// lock
class CGuard
{
    public:
        CGuard(pthread_mutex_t& lock);
        ~CGuard();

    public:
        static void enterCS(pthread_mutex_t& lock);
        static void leaveCS(pthread_mutex_t& lock);

        static void createMutex(pthread_mutex_t& lock);
        static void releaseMutex(pthread_mutex_t& lock);

        static void createCond(pthread_cond_t& cond);
        static void releaseCond(pthread_cond_t& cond);

    private:
        pthread_mutex_t& m_Mutex;            // Alias name of the mutex to be protected
        int m_iLocked;                       // Locking status

        CGuard& operator=(const CGuard&);
};



class CLog
{
public:
    static const int CLOG_LEVEL_API;     
    static const int CLOG_LEVEL_GB_TASK;     

    // log function
    static void log(bool file, const char* format, ...);
    static void log(bool file, int level, const char* format, ...);
    static void log(int level, const char* format, ...);
};


class CRandom
{
public:
    static uint32_t rand();

private:

    /*
     * get random
     */
    static uint32_t get_random();
};


class CStr2Digit
{
public:
    static bool get_lint(char *str, long int &val);
};



#endif  // __COMMON_H__

