
// MT_UIDlg.h : ͷ�ļ�
//

#pragma once
#include "RtpMedia.h"
#include <gb.h>
#include <string>
#include <map>
#include "afxcmn.h"

#define BEGIN_RTP_PORT	11000

// ���ػỰ����״̬��
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

// CMT_UIDlg �Ի���
class CMT_UIDlg : public CDialogEx
{
// ����
public:
	CMT_UIDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CMT_UIDlg();
// �Ի�������
	enum { IDD = IDD_MT_UI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
	EGbclientAdpState							gb_client_adp_state;				// ״̬��
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
