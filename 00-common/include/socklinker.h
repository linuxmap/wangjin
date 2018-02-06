#ifndef _SOCK_LINKER_H_
#define _SOCK_LINKER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/epoll.h>
#include <time.h>

#define SOCKLINK_INVALID_HANDLE     NULL
typedef void *      HSockLinkHandle;


/// epoll deal function
typedef void (*FSockEpollDealFun)(void *context, struct epoll_event *ev);

/// timer task
typedef int (*FSockTimerTask)(void *context, struct timespec ts);

/// epoll data
typedef struct tagSockEpollCtx
{
    FSockEpollDealFun   func;
    void                *context;
}TSockEpollCtx;

/// event task param
typedef struct tagSockEvTaskParam
{
    int                 fd;
    int                 op;
    struct epoll_event  ev;         /// with ev.data.ptr must be type of TSockEpollCtx *
}TSockEvTaskParam;

/*
 * create sock linker
 */
HSockLinkHandle sock_create_linker(int timeout, uint32_t max_events);

/*
 * add epoll event task
 */
int sock_add_ev_task(HSockLinkHandle handle, TSockEvTaskParam *param);

/*
 * add timer task
 */
int sock_add_timer_task(HSockLinkHandle handle, FSockTimerTask func, void *context);


#ifdef __cplusplus
}
#endif

#endif // _SOCK_LINKER_H_
