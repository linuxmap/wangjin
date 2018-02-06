
#include <stdint.h>
#include <pthread.h>

#include "common.h"
#include "gb.h"
#include "gbapi.h"
#include "gbexosip.h"
#include "gb_server.h"



int CGbApi::init()
{
    return 0;
}


int CGbApi::server_start(gb_handle_t handle)
{
    return CGbServer::start(handle);
}

int CGbApi::server_set_ev_cb(gb_handle_t handle, void *context, int (*cb)(void *ctx, int type, void *var, int var_len))
{
    return CGbServer::set_ev_cb(handle, context, cb);
}

int CGbApi::server_invite(gb_handle_t handle, MN::CMediaInvite &invite_param)
{
    return CGbServer::invite(handle, invite_param);
}
