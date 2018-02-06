#ifndef __DEBUG_H__     ///< 头文件宏,防止重包含
#define __DEBUG_H__     ///< 头文件宏,防止重包含


#include <stdint.h>

#define DEBUG_API __attribute__ ((visibility("default")))



namespace DEBUG 
{

struct DEBUG_API TDdbInitParam
{
    uint16_t    port;               // telnet debug port, < 2500:disable telnet debug server, default:2500
    uint32_t    max_reg_func_num;   // default:64
};

/*
 * init debug module
 */
DEBUG_API int debug_init(TDdbInitParam *param   = NULL);

/// deinit telnet debug 
DEBUG_API int debug_deinit();

/// register cmd
DEBUG_API int debug_reg_cmd(const char* name, void* func, const char* usage);

/// debug log output
DEBUG_API int dbg_printf(const char* format, ...);
}











#endif  // __DEBUG_H__
