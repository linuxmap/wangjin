/******************************************************************************
模块名	： OSP
文件名	： ospNodeMan.h
相关文件：
文件实现功能：OSP结点管理功能的主要头文件
作者	：向飞
版本	：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
06/11/2003	2.0          向飞         创建
******************************************************************************/

#ifndef OSP_NODEMAN_H
#define OSP_NODEMAN_H

#include "osp.h"

// 初始态
const u32 IDLE_STATE          = 0;
// 运行态
const u32 RUNNING_STATE       = 1;
// 结点扫描定时器号
const u16 NODE_SCAN_TIMER     = 1;
// 结点扫描定时间隔
const u32 NODE_SCAN_INTERVAL  = 1000;
// 结点扫描超时事件
const u16 NODE_SCAN_TIMEOUT   = 1;
// 启动事件
const u16 START_UP_EVENT      = OSP_POWERON;
// 结点管理App的Id
const u16 NODE_MAN_APPID      = 122;

//结点管理实例类定义
class CNodeManInstance: public CInstance
{
public:
	//入口函数
	void InstanceEntry(CMessage *const pMsg);
    //主动释放Timer，消除内存泄露
	virtual void InstanceExit(void);

private:
	//结点检测
	void NodeScan(void);

private:
	//状态打印计数
	u32 m_dwStatPrtCount;
};

//结点管理App类定义
typedef zTemplate<CNodeManInstance, 1> CNodeManApp;

#endif // OSP_NODEMAN_H