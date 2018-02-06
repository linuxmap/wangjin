
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#ifdef _MSC_VER
#include <winsock2.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#ifdef _MSC_VER

#else
#define WSAGetLastError()       errno
#endif


#define sock_log(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#include "oscbb.h"

namespace CBB
{
/*
* socket create & bind
*/
SOCKHANDLE OSCBB_API cbb_sock_create(int domain, int type, int protocol, int reuse, void *addr, int addr_len)
{
    int ret;

    // create
    SOCKHANDLE fd = socket(domain, type, protocol);
    if (fd == INVALID_SOCKET)
    {
        sock_log("create socket failed,domain:%d,type:%d,protocol:%d,errno:%d\n",
                    domain, type, protocol, WSAGetLastError());
        return -1;
    }

    // reuse
    if (0 != reuse)
    {
#ifdef _MSC_VER
        ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));
#else
        ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
        if (ret < 0)
        {
            sock_log("set sock addr reuse failed,errno:%d\n", errno);
        }
    }

    // bind
    if ((NULL != addr) && (0 != addr_len))
    {
#ifdef _MSC_VER
        ret = bind(fd, (struct sockaddr *)addr, addr_len);
#else
        ret = bind(fd, (struct sockaddr *)addr, (socklen_t)addr_len);
#endif
        if (ret < 0)
        {
            sock_log("socket bind failed,errno:%d\n", errno);
            cbb_sock_close(fd);
            return -1;
        }
    }

    return fd;
}


/*
 * ¹Ø±ÕÌ×½Ó×Ö
 */
int OSCBB_API cbb_sock_close(SOCKHANDLE handle)
{
#ifdef _MSC_VER
    return closesocket(handle);
#else
    return close(handle);
#endif
}

int cbb_socket_nonblock(SOCKHANDLE socket, int enable)
{
#ifdef _MSC_VER
    printf("to set non block\n");
    u_long param = enable;
    return ioctlsocket(socket, FIONBIO, &param);
#else
    if (enable)
      return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
    else
      return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
#endif /* HAVE_WINSOCK2_H */
}


}
