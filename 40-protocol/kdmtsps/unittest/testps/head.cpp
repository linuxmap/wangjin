#include "ps.h"

//²éÕÒ0x000001ÆðÊ¼·û  [OUT]u32 *pdwPostion
static s32 PsReadFindHead(u8 *pbyBuf, u32 dwLen, u32 *pdwPostion)
{
    u32 dwPos = 0;
    
    while ((dwPos + 2) < dwLen)
    {
        if (pbyBuf[dwPos] == 0 && pbyBuf[dwPos+1] == 0 && pbyBuf[dwPos+2] == 1)
        {
			*pdwPostion = dwPos;
            return RPS_OK;
        }		
        dwPos++;
    }	
    return RPS_ERR_STARTCODE;
}
