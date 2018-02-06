
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

typedef enum tagSipResCode
{
    SIP_RESPONSE_NONE               = 0,    /// unkwon code

    // 1xx Provisional
    SIP_RESPONSE_100_TRYING             = 100,  // 100 Trying
    SIP_RESPONSE_180_RINGING            = 180,  // 180 Ringing
    SIP_RESPONSE_181_CALL_FORWARDED     = 181,  // 181 Call Is Being Forwarded
    SIP_RESPONSE_182_CALL_QUEUED        = 182,  // 182 Queued
    SIP_RESPONSE_183_SESSION_PROGRESS   = 183,  // 183 Session Progress

    // 2xx Successful
    SIP_RESPONSE_200_OK         = 200,  // 200 OK
    SIP_RESPONSE_202_ACCEPTED   = 202,  // 202 Acceptedkk

    // 3xx Redirection
    SIP_RESPONSE_300_MULTI_CHOICES      = 300,  // 300 Multiple Choices
    SIP_RESPONSE_301_MOVED_PERMANENTLY  = 301,  // 301 Moved Permanently
    SIP_RESPONSE_302_MOVED_TEMP         = 302,  // 302 Moved Temporarily
    SIP_RESPONSE_305_USE_PROXY          = 305,  // 305 Use Proxy
    SIP_RESPONSE_380_ALTER_SERVICE      = 380,  // 380 Alternative Service

    // 4xx Client Error
    SIP_RESPONSE_400_BAD_REQ                        = 400,  // 400 Bad Request
    SIP_RESPONSE_401_UNAUTHORIZED                   = 401,  // 401 Unauthorized
    SIP_RESPONSE_402_PAYMENT_REQUIRED               = 402,  // 402 Payment Required
    SIP_RESPONSE_403_FORBIDDEN                      = 403,  // 403 Forbidden
    SIP_RESPONSE_404_NOT_FOUND                      = 404,  // 404 Not Found
    SIP_RESPONSE_405_METHOD_NOT_ALLOWED             = 405,  // 405 Method Not Allowd
    SIP_RESPONSE_406_NOT_ACCEPTABLE                 = 406,  // 406 Not Acceptable
    SIP_RESPONSE_407_PROXY_AUTHENTICATION_REQUIRED  = 407,  // 407 Proxy Authentication Required
    SIP_RESPONSE_408_REQ_TIMEOU                     = 408,  // 408 Request Timeout
    SIP_RESPONSE_409_CONFLICT                       = 409,  // 409 Conflict
    SIP_RESPONSE_410_GONE                           = 410,  // 410 Gone
    SIP_RESPONSE_411_LENGTH_REQUIRED                = 411,  // 411 Length Required
    SIP_RESPONSE_413_REQ_ENTITY_TOO_LARGE           = 413,  // 413 Request Entity Too Large
    SIP_RESPONSE_414_REQ_URI_TOO_LARGE              = 414,  // 414 Request-URI Too Long
    SIP_RESPONSE_415_UNSUPPORTED_MEDIA_TYPE         = 415,  // 415 Unsupported Media Type
    SIP_RESPONSE_416_UNSUPPORTED_URI_SCHEME         = 416,  // 416 Unsupported URI Scheme
    SIP_RESPONSE_420_BAD_EXTENSION                  = 420,  // 420 Bad Extension
    SIP_RESPONSE_421_EXTENSION_REQUIRED             = 421,  // 421 Extension Required
    SIP_RESPONSE_422_SESSION_TIMER_INTERV_TOO_SMALL = 422,  // 422 Session Timer Interval Too Small
    SIP_RESPONSE_423_INTERV_TOO_BRIEF               = 423,  // 423 Interval Too Brief
    SIP_RESPONSE_428_USER_AUTHENTICATION_TOKEN      = 428,  // 428 Use Authentication Token
    SIP_RESPONSE_429_PROVIDE_REFERROR_IDENTITY      = 429,  // 429 Provide Referror Identity
    SIP_RESPONSE_480_TEMPORARILY_UNAVAILABLE        = 480,  // 480 Temporarily Unavailable
    SIP_RESPONSE_481_DIALOG_TRANSC_NOT_EXIST        = 481,  // 481 Dialog/Transaction Does Not Exist
    SIP_RESPONSE_482_LOOP_DETECTED                  = 482,  // 482 Loop Detected
    SIP_RESPONSE_483_TOO_MANAY_HOPS                 = 483,  // 483 Too Many Hops
    SIP_RESPONSE_484_ADDRESS_INCOMLETE              = 484,  // 484 Address INcomplete
    SIP_RESPONSE_485_AMBIGUOUS                      = 485,  // 485 Ambiguous
    SIP_RESPONSE_486_BUSY_HERE                      = 486,  // 486 Busy Here
    SIP_RESPONSE_487_REQ_TERMINATED                 = 487,  // 487 Request Terminated
    SIP_RESPONSE_488_NOT_ACCEPTABLE_HERE            = 488,  // 488 Not Acceptable Here
    SIP_RESPONSE_489_BAD_EVENT                      = 489,  // 489 Bad Event
    sip_response_491_request_pending                = 491,  // 491 request pending

    // 5xx Server Error
    SIP_RESPONSE_500_SERVER_INTERNAL_ERR    = 500,  // 500 Server Internal Error
    SIP_RESPONSE_501_NOT_IMPLEMENTED        = 501,  // 501 Not Implemented
    SIP_RESPONSE_502_BAD_GATEWAY            = 502,  // 502 Bad Gateway
    SIP_RESPONSE_503_SERVEICE_UNAVAILABEL   = 503,  // 503 Service Unavailable
    SIP_RESPONSE_504_GATEWAY_TIMEOUT        = 504,  // 504 Gateway Timeout
    SIP_RESPONSE_505_VERSION_NOT_SUPPORTED  = 505,  // 505 Version Not Supported
    SIP_RESPONSE_513_MESSAGE_TOO_LARGE      = 513,  // 513 Message Too Large

    // 6xx Global Error
    SIP_RESPONSE_600_BUSY_EVERYWHRER    = 600,  // 600 Busy Everywhere
    SIP_RESPONSE_603_DECLINET           = 603,  // 603 Decline
    SIP_RESPONSE_604_NOT_EXIST_ANYWHERE = 604,  // 604 Does Not Exist Anywhere
    SIP_RESPONSE_606_NOT_ACCEPTABLE     = 606,  // 606 Not Acceptable
}ESipResCode;

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


class CTime
{
public:
    static uint64_t gettime_ms(void);
};

#ifdef _MSC_VER
int strcasecmp(char *s1, char *s2);
#define strncasecmp  strnicmp

#endif

void gb_write(char *szFormat, ...);


#endif  /// __COMMON_H__
