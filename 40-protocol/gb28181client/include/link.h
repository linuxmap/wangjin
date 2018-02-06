
#ifndef __LINK_H__
#define __LINK_H__

#include "oscbb.h"
#include <stdint.h>
#include <thread>

class CLink
{
public:
    typedef void(*FInetDataCb)(void *context, void *buf, int buf_len, uint32_t rmt_ip, uint16_t rmt_port, uint64_t cur_time);
    typedef void(*FUnixDataCb)(void *context, void *buf, int buf_len, char *rmt_addr, uint64_t cur_time);

    typedef void(*FEvenTrigger)(void *context, uint64_t cur_time);

    /// ���ṹ���������ַ��Ϊ������
    struct CCfgParam
    {
        CCfgParam(){};
        uint32_t        client_addr;               /// ����ͻ��˵�ַ
        uint16_t        client_port;               /// ����ͻ��˶˿�
        FInetDataCb     client_data_cb;         /// ����ص�
        void            *client_data_cb_ctx;    /// ����ص��û�������
#if 0
        char            inter_msg_addr[64];     /// �ڲ�ͨ�Ŷ˿�,�����ڲ�ʹ��UNIX��UDPͨ�ŷ�ʽ
        FUnixDataCb     inter_msg_cb;           /// �ڲ���Ϣ�ص�
        void            *inter_msg_cb_ctx;      /// �ڲ���Ϣ�ص��û�������

        uint32_t        external_addr;          /// �����������������ͨ�ŵ�ַ
        uint16_t        external_port;          /// �����������������ͨ�Ŷ˿�
        FInetDataCb     external_data_cb;       /// ����ص�
        void            *external_data_cb_ctx;  /// ����ص��û�������
#endif

        FEvenTrigger    event_trigger_cb;       /// ������
        void            *ev_trigger_ctx;        /// �������û�������
    };

    /// ���캯��
    CLink();

    /*
    * create sock linker
    */
    int create(CCfgParam &param);

    /// �����������Ϣ
    int sip_send_data(void *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /// �ڲ�ͨ�ŷ�����Ϣ
    int inter_send_data(char *buf, int len, char *rmt_addr);

    /// ��������ͨ��������Ϣ
    int external_send_data(char *buf, int len, uint32_t rmt_ip, uint16_t rmt_port);

    /*
    * destroy link
    */
    int destroy();

    /// routine
    static void *sock_routine(void *arg);

private:

    void deal_trigger();

    void deal_sock();

    /// thread module
    std::thread     *link_task_handle;

    SOCKHANDLE		 sip_fd;
    SOCKHANDLE		 inter_fd;
    SOCKHANDLE		external_fd;

    unsigned int    timeout;        /// ��λ:ms
    uint64_t        cur_time;

    char            recv_buf[8192];     /// �������buffer

    FInetDataCb     sip_data_cb;            /// sip��Ϣ�ص�����
    void            *sip_cb_ctx;            /// sip��Ϣ�ص��û�����
    FEvenTrigger    trigger_cb;             /// �¼�������
    void            *trigger_ctx;           /// ����������
    uint64_t        trigger_count;
};


#endif // __LINK_H__
