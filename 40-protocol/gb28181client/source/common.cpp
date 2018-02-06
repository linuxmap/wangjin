

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#ifndef _MSC_VER

#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/time.h>

#else
#endif


#include <time.h>


#include <oscbb.h>

#include "common.h"
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

#ifndef _MSC_VER
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

    // 引入随机�?
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
#else

/*
 * get random
 */
uint32_t CRandom::get_random()
{
    uint32_t ret;

    // 引入随机�?
    ::srand((unsigned int)::time(NULL));
    ret = ::rand();

    if (0 == ret)
    {
        ret = (uint32_t)&ret;
    }

    return ret;
}

#endif



/*
 * get long int from str
 */
bool CStr2Digit::get_lint(char *str, long int &val)
{
    errno = 0;    /* To distinguish success/failure after call */
    char *endptr;
    val = strtol(str, &endptr, 10);

    /* Check for various possible errors */
#ifndef _MSC_VER
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
                || (errno != 0 && val == 0)) {
        return false;
    }
#endif
    if (endptr == str) {
        return false;
    }

    return true;
}


void gb_write(char *szFormat, ...)
{
    char szMsg[8192] = { 0 };
    struct tm *now;
    time_t curtime;
    int nlen;
    va_list pvlist;
    int  nstrLen;

    time(&curtime);
    now = localtime(&curtime);
    nlen = sprintf(szMsg, "%d-%d-%d %2d:%2d:%2d ",
        now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

    va_start(pvlist, szFormat);
    nstrLen = vsprintf(szMsg + nlen, szFormat, pvlist);
    if (nstrLen <= 0 || nstrLen >= 8192)
    {
        va_end(pvlist);
        return;
    }
    va_end(pvlist);

    FILE *pFile;
    char achLogPathName[255];
    sprintf(achLogPathName, "gb_write.log");

    pFile = fopen(achLogPathName, "a+");
    if (pFile != NULL)
    {
        fseek(pFile, 0, SEEK_END);
        ftell(pFile);       
    }
   
    fputs(szMsg, pFile);
    fclose(pFile);
}



