#ifndef __GB_ADAPTER_H__
#define __GB_ADAPTER_H__


class CGbAdpUdp;

class CGbAdp
{
public:
    int init(bool udp = true);




private:
    CGbAdpUdp *udp_adp;
};


#endif  // __GB_ADAPTER_H__
