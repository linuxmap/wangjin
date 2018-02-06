

#ifndef __NVS_CORE_H__
#define __NVS_CORE_H__


class CNvsCore
{
public:
    static void deal_msg(void *ctx, MN::CMsgFormat &msg, uint64_t time);

    /*
     * deal msg @MN_GB_RMT_RES_RECORDINFO
     */
    static int deal_rmt_res_recordinfo(void *ctx, MN::CMsgFormat &msg, uint64_t time);
};



#endif  /// __NVS_CORE_H__
