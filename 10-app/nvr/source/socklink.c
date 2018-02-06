#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/un.h>

#include "socklinker.h"

#define sock_log(fmt, ...)  printf("%s %d:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

typedef struct tagSockCreateParam
{
    int domain;
    int type;
    int protocol;

    int reuse;

    int local_addr_len;
    int rmt_addr_len;
    union tagLinkAddr
    {
        struct sockaddr addr_sock;
        struct sockaddr_in addr_in;
        struct sockaddr_un addr_un;
    }local_addr, rmt_addr;
}TSockCreateParam;

typedef struct tagSockLink
{
    int                 ep_fd;
    int                 timeout;
    uint32_t            max_events;  // max events

    int                 num;
    struct epoll_event  *ev;

    int                 run;
    int                 exit;
    pthread_t           pid;

    // cur ts
    struct timespec     ts;
}TSockLink;

/*
 * socket create & bind
 */
int sock_create(TSockCreateParam *param)
{
    int ret;

    // create
    int fd = socket(param->domain, param->type, param->protocol);
    if (fd < 0)
    {
        sock_log("create socket failed,domain:%d,type:%d,protocol:%d,errno:%d\n",
                    param->domain, param->type, param->protocol, errno);
        return -1;
    }

    // reuse
    if (0 != param->reuse)
    {
        ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &param->reuse, sizeof(param->reuse));
        if (ret < 0)
        {
            sock_log("set sock addr reuse failed,errno:%d\n", errno);
        }
    }

    // bind
    if (param->local_addr_len > 0)
    {
        ret = bind(fd, &param->local_addr.addr_sock, param->local_addr_len);
        if (ret < 0)
        {
            sock_log("socket bind failed,errno:%d\n", errno);
            close(fd);
            return -1;
        }
    }

    return 0;
}


#define sock_link_max(a, b)  ((a) > (b) ? (a) : (b))
#define sock_link_min(a, b)  ((a) > (b) ? (b) : (a))

/*
 * create sock linker
 */
HSockLinkHandle sock_create_linker(int timeout, uint32_t max_events)
{
    TSockLink *link;
    int fd, ret;
#define SOCKLINK_MAX_EVENTS     1024
#define SOCKLINK_MIN_EVENTS     4
#define SOCKLINK_MIN_TIMEOUT    10
    max_events = sock_link_max(max_events, SOCKLINK_MIN_EVENTS);
    max_events = sock_link_min(max_events, SOCKLINK_MAX_EVENTS);

    if (0 != timeout)
    {
        timeout = sock_link_max(timeout, SOCKLINK_MIN_TIMEOUT);
    }

    fd = epoll_create1(0);
    if (fd < 0)
    {
        sock_log("create epoll failed,flag:%d,errno:%d\n", 0, errno);
        return SOCKLINK_INVALID_HANDLE;
    }

    link = (TSockLink *)malloc(sizeof(*link));
    if (NULL == link)
    {
        sock_log("malloc failed, no mem\n");
        close(fd);
        return SOCKLINK_INVALID_HANDLE;
    }

    link->ev = (struct epoll_event *)malloc(sizeof(struct epoll_event) * max_events);
    if (NULL == link->ev)
    {
        sock_log("malloc failed, no mem, events num:%u\n", max_events);
        close(fd);
        free(link);
        return SOCKLINK_INVALID_HANDLE;
    }

    link->ep_fd     = fd;
    link->timeout   = timeout;
    link->max_events    = max_events;
    sock_log("link:%p create,fd:%d,timeout:%d,max events:%u\n", link, fd, timeout, max_events);

void *sock_runtine(void *arg);
    ret = pthread_create(&link->pid, NULL, sock_runtine, link);
    if (0 != ret)
    {
        sock_log("create run task failed, ret:%d\n", ret);
    }

    return link;
}

/*
 * destroy sock linker
 */
int sock_delete_linker(TSockLink *link)
{
    int ret = close(link->ep_fd);
    if (ret < 0)
    {
        sock_log("link:%p close fd:%d failed,errno:%d\n", link, link->ep_fd, errno);
    }
    free(link->ev);
    free(link);
    sock_log("link:%p close fd:%d, delete now\n", link, link->ep_fd);

    return 0;
}
#if 0
/*
 * deal timer on link
 */
void sock_deal_timer(TSockLink *link)
{
    int ret;
    uint32_t i;

    // get time
    ret = clock_gettime(CLOCK_MONOTONIC, &link->ts);
    if (ret < 0)
    {
        sock_log("get clock time failed,errno%d\n", errno);
    }

    // add
    for(i = 0; i < link->timer_func_num; i++)
    {
        if (NULL == link->func[i])
        {
            continue;
        }

        ret = link->func[i](link->context[i], link->ts);
        if (ret < 0)
        {
            sock_log("set func[%d] with NULL\n", i);
            link->func[i] = NULL;
        }
    }
}
#endif

/*
 * deal epoll events
 */
void sock_deal_events(TSockLink *link)
{
    int i;
    struct epoll_event ev;
    TSockEpollCtx *ctx;

    // sock_log("deal epoll events,num:%d\n", link->num);

    // deal come data
    for (i = 0; i < link->num; i++)
    {
        ev = *(link->ev + i);           // copy avoid mangle
        if (NULL != ev.data.ptr)
        {
            ctx = ev.data.ptr;
            ctx->func(ctx->context, &ev);
        }
        else
        {
           sock_log("fatal error happened, epoll event with no user\n");
        }
    }
}

/*
 * epoll task
 */
void *sock_runtine(void *arg)
{
    int ret;
    TSockLink *link = (TSockLink *)arg;         // cur link
    struct epoll_event *ev = link->ev;

    // run & not exit
    link->run   = 1;
    link->exit  = 0;

    // start service
    sock_log("link:%p->%d running\n", link, link->ep_fd);
    while(1)
    {
        if (0 != link->exit)
        {
            sock_log("link:%p->%d is commanded to exit\n", link, link->ep_fd);
            break;
        }

        ret = epoll_wait(link->ep_fd, ev, link->max_events, link->timeout);
        if (ret < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }

            sock_log("link:%p->%d epoll wait failed, task to exit, errno:%d\n", link, link->ep_fd, errno);
            break;
        }
        else if (0 == ret)
        {
            continue;
        }

        // deal event
        link->num   = ret;
        sock_deal_events(link);

    }

    sock_log("link:%p->%d task exit now\n", link, link->ep_fd);
    link->run   = 0;    // task now not run
    link->exit  = 1;

    return NULL;
}

/*
 * add epoll event task
 */
int sock_add_ev_task(HSockLinkHandle handle, TSockEvTaskParam *param)
{
    int ret;
    TSockLink *link = handle;

    ret = epoll_ctl(link->ep_fd, param->op, param->fd, &param->ev);
    if (ret < 0)
    {
        sock_log("sock add ev task failed,ep fd:%d,fd:%d,errno:%d\n",
                    link->ep_fd, param->fd, errno);
        return -1;
    }

    return 0;
}

/*
 * del epoll event task
 */
int sock_del_ev_task(HSockLinkHandle handle, TSockEvTaskParam *param)
{
    int ret;
    TSockLink *link = handle;

    ret = epoll_ctl(link->ep_fd, param->op, param->fd, &param->ev);
    if (ret < 0)
    {
        sock_log("sock del ev task failed,errno:%d\n", errno);
        return -1;
    }

    return 0;
}
#if 0
/*
 * add timer task
 */
int sock_add_timer_task(HSockLinkHandle handle, FSockTimerTask func, void *context)
{
    int i, ret = -1;
    TSockLink *link = handle;

    // add
    for(i = 0; i < SOCKLINK_MAX_TIMER_FUNC_NUM; i++)
    {
        if (NULL == link->func[i]) 
        {
            // add
            link->func[i]       = func;
            link->context[i]    = context;
            link->timer_func_num++;
            ret = 0;
            break;
        }
    }

    sock_log("add timer task ret:%d\n", ret);
    return 0;
}

/*
 * del timer task
 */
int sock_del_timer_task(HSockLinkHandle handle, FSockTimerTask func, void *context)
{
    int i, ret = -1;
    TSockLink *link = handle;

    // add
    for(i = 0; i < SOCKLINK_MAX_TIMER_FUNC_NUM; i++)
    {
        if ( (NULL != link->func[i]) && (link->func[i] == func) ) 
        {
            link->func[i]       = NULL;
            link->context[i]    = NULL;
            link->timer_func_num--;
            ret = 0;
            break;
        }
    }

    sock_log("del timer task ret:%d,context:%p\n", ret, context);
    return ret;
}
#endif
