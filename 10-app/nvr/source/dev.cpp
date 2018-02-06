
#include "stdlib.h"
#include "stdio.h"
#include <unistd.h>
#include <sys/prctl.h>
#include <errno.h>
#include <string.h>

#include "oscbb.h"
#include "debug.h"
#include "msgno.h"
#include "item.h"
#include "dev.h"
#include "item_rec.h"

/// module log hander
#define dev_log(fmt, ...) printf("[DEV] %s %u:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/// DEV namespace
namespace DEV 
{

/// 设备资源条目组
class CDevItem
{
public:
    CDevItem();
    static void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    static void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }

    /// 初始化
    static int init();

    /// 执行程序
    static void *routine(void *);

    /// 设备增加
    static int dev_add(MN::CDevRegister &dev_reg);

private:

    std::string  device_id;
    std::string  name;
    std::string  ip;
    std::string  port;

    int     sub_item_num;

    bool    valid;
    bool    on;
    bool    have_stream;
    bool    playing;

    class CDevItem *next_sibling;
    class CDevItem *child;
    class CDevItem *parent;

    static  pthread_t       pid;
    static pthread_mutex_t  mutex;
};

pthread_mutex_t  CDevItem::mutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_t       CDevItem::pid;


/// max dev num
#define DEV_ITEM_NUM    32
static CDevItem dev_item_table[DEV_ITEM_NUM];


CDevItem::CDevItem()
{
    valid = false;
}

int CDevItem::init()
{
    int ret;

    /// register cmd
    ret = DEBUG::debug_reg_cmd("resshow", (void *)CResouceItem::show_all_item, "show all resource item");
    if (0 != ret)
    {
        printf("dev module reg cmd failed,ret:%d", ret);
    }

    if (false == CDevRecordItem::create())
    {
        dev_log("record item create failed\n");
        return -1;
    }

    ret = pthread_create(&CDevItem::pid, NULL, routine, NULL);
    if (0 != ret)
    {
        printf("create thread failed,errno:%d", ret);
    }

    return -ret;
}


int CDevItem::dev_add(MN::CDevRegister &dev_reg)
{
    int i;
    for (i = 0; i < DEV_ITEM_NUM; i++)
    {
        if (false == dev_item_table[i].valid)
        {
            dev_item_table[i].valid = true;
            dev_item_table[i].device_id.assign(dev_reg.device_id);
            dev_item_table[i].name.assign(dev_reg.name);
            dev_item_table[i].ip.assign(dev_reg.ip);
            dev_item_table[i].port.assign(dev_reg.port);

            dev_item_table[i].sub_item_num      = 0;
            dev_item_table[i].on                = false;    // dev_reg.on;
            dev_item_table[i].have_stream       = false;    // dev_reg.have_stream;
            dev_item_table[i].playing           = false;
            dev_item_table[i].next_sibling      = NULL;
            dev_item_table[i].child             = NULL;
            dev_item_table[i].parent            = NULL;

            dev_log("dev add,index:%d\n", i);
            break;
        }
    }

    if (DEV_ITEM_NUM == i)
    {
        dev_log("dev add,index:%d\n", i);
        return -1;
    }

    return 0;
}

void *CDevItem::routine(void *)
{
    printf("dev mgr routine run now\n");

    int ret = prctl(PR_SET_NAME, "DevShedule", NULL, NULL, NULL);
    if (ret < 0)
    {
        dev_log("set dev mgr task name failed,errno:%d\n", errno);
    }

    while(true)
    {
        CDevItem::lock();
        CResouceItem::run(CBB::gettime_ms());
        CDevItem::unlock();

        sleep(1);
    }

    return NULL;
}



// dev_mgr init function
int dev_init(CDevCfgParam &cfg_param)
{
    int ret = CResouceItem::init(cfg_param.max_item_num, cfg_param.media_recv_addr);
    if (0 != ret) 
    {
        dev_log("item init failed,ret:%d\n", ret);
        return -1;
    }

    /// dev_item init
    ret = CDevItem::init();
    if (0 != ret)
    {
        dev_log("dev item init failed,ret:%d\n", ret);
        return -1;
    }

    return 0;
}

int dev_add(MN::CDevRegister &dev_reg)
{
    return CDevItem::dev_add(dev_reg);
}

int dev_del(MN::CDevRegister &dev_reg)
{
    return 0;   /// CDevApi::dev_del(dev_reg);
}

void *dev_malloc(size_t size)
{
    return malloc(size);
}
void *dev_mallocz(size_t size)
{
    void *p = malloc(size);
    if (p)
    {
        memset(p, 0, size);
    }
    return p;
}
void dev_free(void *p)
{
    free(p);
}

}
