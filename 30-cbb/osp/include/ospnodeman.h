/******************************************************************************
ģ����	�� OSP
�ļ���	�� ospNodeMan.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP�������ܵ���Ҫͷ�ļ�
����	�����
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
06/11/2003	2.0          ���         ����
******************************************************************************/

#ifndef OSP_NODEMAN_H
#define OSP_NODEMAN_H

#include "osp.h"

// ��ʼ̬
const u32 IDLE_STATE          = 0;
// ����̬
const u32 RUNNING_STATE       = 1;
// ���ɨ�趨ʱ����
const u16 NODE_SCAN_TIMER     = 1;
// ���ɨ�趨ʱ���
const u32 NODE_SCAN_INTERVAL  = 1000;
// ���ɨ�賬ʱ�¼�
const u16 NODE_SCAN_TIMEOUT   = 1;
// �����¼�
const u16 START_UP_EVENT      = OSP_POWERON;
// ������App��Id
const u16 NODE_MAN_APPID      = 122;

//������ʵ���ඨ��
class CNodeManInstance: public CInstance
{
public:
	//��ں���
	void InstanceEntry(CMessage *const pMsg);
    //�����ͷ�Timer�������ڴ�й¶
	virtual void InstanceExit(void);

private:
	//�����
	void NodeScan(void);

private:
	//״̬��ӡ����
	u32 m_dwStatPrtCount;
};

//������App�ඨ��
typedef zTemplate<CNodeManInstance, 1> CNodeManApp;

#endif // OSP_NODEMAN_H