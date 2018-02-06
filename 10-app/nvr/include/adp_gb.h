#ifndef __ADAPTER_GB_H__
#define __ADAPTER_GB_H__
#include "cfg.h"

class CAdpGbParam;



class CAdpGb
{
public:
    /// init AdpGb
    static int init(CFG::CCfgParam &param);

    /// deinit AdpGb
    static int deinit();
    
    /// get ogj
    static CAdpGb *get_instance();

private:
    CAdpGb();
    CAdpGbParam *param;
};




#endif  // __ADAPTER_GB_H__

