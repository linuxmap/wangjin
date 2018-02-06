
// MT_UIDlg.h : 头文件
//

#pragma once
#include "RtpMedia.h"
#include <gb.h>
#include <string>
#include <map>
#include "afxcmn.h"

#define BEGIN_RTP_PORT	11000

// 本地会话信令状态机
enum EGbclientAdpState
{
	ECLIENT_ADP_STATE_INVALID,
	ECLIENT_ADP_STATE_INIT,
	ECLIENT_ADP_STATE_REGISTING,
	ECLIENT_ADP_STATE_REGISTED,
	ECLIENT_ADP_STATE_QUERYING_CATALOG,
	ECLIENT_ADP_STATE_QUERYED_CATALOG,
};

struct PlayParam
{
	HWND		mhPlayWnd;
	CRtpMedia*	mpRtpMedia;
};

// CMT_UIDlg 对话框
class CMT_UIDlg : public CDialogEx
{
// 构造
public:
	CMT_UIDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CMT_UIDlg();
// 对话框数据
	enum { IDD = IDD_MT_UI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	int OnGBMessageCallBack(int type, void *var, int var_len);
private:
	gb_handle_t									gb_client_handle;
	EGbclientAdpState							gb_client_adp_state;				// 状态机
	std::map<std::string,GB::CGbCatalogItem*>	mMapCatalogItems;
	CImageList									mImageList;
	CTreeCtrl									mCatalogList;
	uint16_t									mnBeginRtpPort;
	PlayParam									mtPlayParam[9];
private:
	void InviteMedia(char* device_id);
public:
	afx_msg void OnClose();
	afx_msg void OnNMCustomdrawTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);

};
