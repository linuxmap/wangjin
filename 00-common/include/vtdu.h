#ifndef __VTDU_H__
#define __VTDU_H__

#define VTDU_API __attribute__ ((visibility("default")))

namespace VTDU
{
    /// 消息中心服务配置参数
    struct VTDU_API CVtduCfgParam
    {
        CVtduCfgParam();

        uint32_t    channel_num;
        int         inter_fd;
    };

    /// 媒体分发模块初始化
    VTDU_API int vtdu_init(const CVtduCfgParam &param);

}










#endif //  __VTDU_H__
