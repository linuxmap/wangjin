#ifndef __NCPROXY_SESSION_H__
#define __NCPROXY_SESSION_H__

int ncs_init();


// nc session set event callback
void ncs_set_ev_cb(FNcproxyEvCb cb, void *context);

/*
 * reply nc 
 */
void ncs_reply(void *context, void *buf, int len);

#endif  // __NCPROXY_SESSION_H__
