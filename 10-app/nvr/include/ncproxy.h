#ifndef __NCPROXY_H__
#define __NCPROXY_H__

#define NCPROXY_INVALID_HANDLE     NULL
typedef void *      HNcproxyHandle;

/// analysis event
typedef enum tagNcproxyEvent{
    NCE_GET_NVR_VERSION     = 1,        /// get nvr version


    NCE_GET_GB_DEV_NUM      = 1000,
    NCE_GET_GB_DEV_LIST     = 1001,       /// get gb28181 registered device list



    NCE_REQ_DEV_STREAM      = 2000,       /// get gb28181 registered device list
}ENcproxyEvent;






/// analysis event callback
typedef void (*FNcproxyEvCb)(void *context, ENcproxyEvent ev, void *buf, int len);





/*
 * init nc proxy for incoming nc connections
 */
HNcproxyHandle ncproxy_init(FNcproxyEvCb cb, void *context);

/*
 * set event callback
 */
void ncproxy_set_ev_cb(HNcproxyHandle handle, FNcproxyEvCb cb, void *context);

/*
 * reply nc 
 */
void ncp_reply(void *context, void *buf, int len);


#endif  //  __NCPROXY_H__
