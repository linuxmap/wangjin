#ifndef __GB_API_H__
#define __GB_API_H__





class CGbApi
{
public:
    static int init();

    static int server_create(gb_handle_t *handle);

    static int server_start(gb_handle_t handle);

    static int server_set_ev_cb(gb_handle_t handle, void *context, int (*cb)(void *ctx, int type, void *var, int var_len));

    static int server_invite(gb_handle_t handle, MN::CMediaInvite &invite_param);
};

#endif  //  __GB_API_H__
