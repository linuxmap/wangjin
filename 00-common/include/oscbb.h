#ifndef __OS_CBB_H__     ///< 头文件宏,防止重包含
#define __OS_CBB_H__     ///< 头文件宏,防止重包含

#include <stdint.h>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
/// 头文件定义
#include <windows.h>
#include <winsock2.h>

// 预定义宏

#define OSCBB_API __declspec(dllexport)


#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0600




/// 套接字句柄定义
#define     SEMHANDLE           HANDLE
#define     SOCKHANDLE			SOCKET
#define     TASKHANDLE			HANDLE
#define     TASKID              DWORD
typedef     DWORD(*TASKROUTE)(LPVOID);


/// win32 套接字库文件
#pragma comment(lib,"Ws2_32.lib")

#else
#include <pthread.h>

#define OSCBB_API __attribute__ ((visibility("default")))

#define     SEMHANDLE           pthread_mutex_t
#define     SOCKHANDLE		    int
#define     INVALID_SOCKET      (-1)
#define     TASKHANDLE			pthread_t
#define     TASKID              pthread_t
typedef     void *(*TASKROUTE)(void *);

#define SOCKADDR_IN           struct sockaddr_in
#ifndef SOCKADDR
#define SOCKADDR          struct sockaddr
#endif

#define SOCKET_ERROR          (-1)

#endif


#define CBB_SET_LIMIT(value, max, min)  if ((value) > (max)) (value) = (max);   if ((value) < (min)) ((value) = (min));

#define CBBMAX(a,b) ((a) > (b) ? (a) : (b))
#define CBBMAX3(a,b,c) CBBMAX(CBBMAX(a,b),c)

#define CBBMIN(a,b) ((a) > (b) ? (b) : (a))
#define CBBMIN3(a,b,c) CBBMIN(CBBMIN(a,b),c)

#define CBB_SAFE_DELETE(p)         {if (p) delete(p);p = NULL;}

typedef void*   HOsMutex;
typedef void*   HOscbbHandle;

namespace CBB
{

    /*
    * convert str to long integer
    */
    OSCBB_API bool cbb_str2l(char *str, long int &val);

    /*
    * delay specified millisecond
    */
    void OSCBB_API cbb_delay(int ms);

    /*
    * miliseconds
    */
    uint64_t OSCBB_API gettime_ms(void);

    /*
    * safe send
    */
    int OSCBB_API cbb_send(SOCKHANDLE fd, void *buf, int len, int flags = 0, bool block = false);

    /*
    * recv data
    */
    int OSCBB_API cbb_recv(SOCKHANDLE fd, void *buf, int len, int flags = 0);

    /*
    * safe send
    */
    int OSCBB_API cbb_sendto(SOCKHANDLE fd, void *buf, int len, const void *addr, int addr_len, int flags = 0);

    /*
    * recv data
    */
    int OSCBB_API cbb_recvfrom(SOCKHANDLE fd, void *buf, int buf_len, void  *addr = NULL, int *addr_len = NULL, int flags = 0);
#if 0
    /*
    * create mutex
    */
    int OSCBB_API cbb_mutex_create(HOsMutex *mutex);

    /*
    * delete mutex
    */
    int OSCBB_API cbb_mutex_delete(HOsMutex mutex);

    /*
    * lock mutex
    */
    int OSCBB_API cbb_mutex_lock(HOsMutex mutex);

    /*
    * unlock mutex
    */
    int OSCBB_API cbb_mutex_unlock(HOsMutex mutex);
#endif
    /*
    * 创建调度器
    */
    int OSCBB_API cbb_create_timer_task(HOscbbHandle *handle, uint32_t task_num = 16);

    /*
    * 增加调度任务
    */
    int OSCBB_API cbb_timer_task_add(HOscbbHandle handle, uint32_t sche_period, void(*routine)(void *arg), void *ctx = NULL, uint32_t cnt = 0);

    /*
    * 增加调度任务
    */
    int OSCBB_API cbb_timer_task_del(HOscbbHandle handle, uint32_t sche_period, void(*routine)(void *arg), void *ctx = NULL, uint32_t cnt = 0);

    /*
    * 执行调度器
    */
    int OSCBB_API cbb_timer_task_sche(HOscbbHandle handle, uint64_t cur_time);

    /*
    * socket create & bind
    */
    SOCKHANDLE OSCBB_API cbb_sock_create(int domain, int type, int protocol, int reuse, void *addr, int addr_len);

    /*
    * 关闭套接字
    */
    int OSCBB_API cbb_sock_close(SOCKHANDLE handle);

    /*
     * 设置套接字阻塞属性
     */
    int OSCBB_API cbb_socket_nonblock(SOCKHANDLE socket, int enable);

    /*
    * 创建任务
    * linux:task_id must not set
    */
    int OSCBB_API cbb_task_create(TASKHANDLE &task_handle, TASKROUTE task_entry, void *param, TASKID *task_id = NULL);

    /*
     * create mutex
     * linux:pthread_mutex_t, Windows:HANDLE
     */
    int OSCBB_API cbb_semb_create(SEMHANDLE &sem);

    /*
     * lock 
     * linux:pthread_mutex_t, Windows:HANDLE
     */
    int OSCBB_API cbb_sem_take(SEMHANDLE &hSema);

    /*
     * unlock 
     * linux:pthread_mutex_t, Windows:HANDLE
     */
    int OSCBB_API cbb_sem_give(SEMHANDLE &hSema);

    /*
     * delay
     * linux:usleep, Windows:Sleep
     */
    void OSCBB_API cbb_task_delay(uint32_t ms);

    /*
     * delay
     * linux:usleep, Windows:Sleep
     */
    uint32_t OSCBB_API cbb_clk_rate_get();

    /*
     * returns the number of clock ticks
     */
    uint32_t OSCBB_API cbb_tick_get();
}



#endif  // __OS_CBB_H__     

