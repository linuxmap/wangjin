
// MT_UIDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MT_UI.h"
#include "MT_UIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMT_UIDlg �Ի���




CMT_UIDlg::CMT_UIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMT_UIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	CRtpMedia::Init();
}

CMT_UIDlg::~CMT_UIDlg()
{
	CRtpMedia::UnInit();
}

void CMT_UIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, mCatalogList);
}

BEGIN_MESSAGE_MAP(CMT_UIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMT_UIDlg::OnBnClickedButton1)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CMT_UIDlg::OnNMDblclkTree1)
END_MESSAGE_MAP()


// CMT_UIDlg ��Ϣ�������

BOOL CMT_UIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	mImageList.Create(16,16,0,2,2);
	mImageList.SetBkColor (RGB(255,255,255));
	mImageList.Add(AfxGetApp()->LoadIcon (IDI_ICON1));
	mImageList.Add(AfxGetApp()->LoadIcon (IDI_ICON2));
	mCatalogList.SetImageList(&mImageList,TVSIL_NORMAL);
	// ����Ĳ�����Ϣ
	SetDlgItemText(IDC_GB_ADDR,"10.10.10.101");
	SetDlgItemText(IDC_SERVICE_ID,"42010000002000000012");
	SetDlgItemText(IDC_GB_PORT,"5061");
	SetDlgItemText(IDC_GB_DEMON_ID,"4201000");
	SetDlgItemText(IDC_SERVICE_PWD,"123456");
	SetDlgItemText(IDC_INTERVAL,"60000");
	SetDlgItemText(IDC_MAXCNT,"5");

	// ���صĲ�����Ϣ
	SetDlgItemText(IDC_LOCAL_ADDR,"10.10.10.104");
	SetDlgItemText(IDC_LOCAL_SERVICE_ID,"42010000002000000088");
	SetDlgItemText(IDC_LOCAL_PORT,"5060");
	SetDlgItemText(IDC_LOCAL_DEMON_ID,"4201000");
	SetDlgItemText(IDC_LOCAL_PROXY,"NETMARCH/IMOS");
	SetDlgItemText(IDC_SESS_TIME,"60000");

	mnBeginRtpPort = BEGIN_RTP_PORT;
	
	mtPlayParam[0].mpRtpMedia = NULL;
	mtPlayParam[0].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN001)->GetSafeHwnd();
	mtPlayParam[1].mpRtpMedia = 0;
	mtPlayParam[1].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN002)->GetSafeHwnd();
	mtPlayParam[2].mpRtpMedia = 0;
	mtPlayParam[2].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN003)->GetSafeHwnd();
	mtPlayParam[3].mpRtpMedia = 0;
	mtPlayParam[3].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN004)->GetSafeHwnd();
	mtPlayParam[4].mpRtpMedia = NULL;
	mtPlayParam[4].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN005)->GetSafeHwnd();
	mtPlayParam[5].mpRtpMedia = 0;
	mtPlayParam[5].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN006)->GetSafeHwnd();
	mtPlayParam[6].mpRtpMedia = 0;
	mtPlayParam[6].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN007)->GetSafeHwnd();
	mtPlayParam[7].mpRtpMedia = 0;
	mtPlayParam[7].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN008)->GetSafeHwnd();
	mtPlayParam[8].mpRtpMedia = NULL;
	mtPlayParam[8].mhPlayWnd = GetDlgItem(IDC_STATIC_SCREEN009)->GetSafeHwnd();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMT_UIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMT_UIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMT_UIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int GBMessageHandler(void *ctx, int type, void *var, int var_len)
{
	CMT_UIDlg* pThis = (CMT_UIDlg*)ctx;
	return pThis->OnGBMessageCallBack(type,var,var_len);
}

int CMT_UIDlg::OnGBMessageCallBack(int type, void *var, int var_len)
{
	int nRet = -1;
	switch(type)
	{
		case GB::ECLIENT_EVT_REGISTERED:
			if (ECLIENT_ADP_STATE_REGISTING == gb_client_adp_state)
			{
				gb_client_adp_state = ECLIENT_ADP_STATE_REGISTED;
				MessageBox("��ϲ����ϲ��ע��ɹ���");

				nRet = GB::gb_client_query_catalog(gb_client_handle);
				if (GB_SUCCESS != nRet)
				{
					MessageBox("������Դʧ�ܣ�");
				}
				else
				{
					gb_client_adp_state = ECLIENT_ADP_STATE_QUERYING_CATALOG;
				}
			}
			break;
		case GB::ECLIENT_EVT_RES_CATALOG:
			{
				GB::CGbCatalogItem* item = (GB::CGbCatalogItem*)var;

				std::string sName(item->device_id);
				std::map<std::string,GB::CGbCatalogItem*>::iterator iter = mMapCatalogItems.find(sName);
				if(iter == mMapCatalogItems.end())
				{
					GB::CGbCatalogItem* pNewItem = new GB::CGbCatalogItem;
					memcpy(pNewItem,item,sizeof(GB::CGbCatalogItem));
					mMapCatalogItems.insert(std::pair<std::string,GB::CGbCatalogItem*>(sName,pNewItem));

					HTREEITEM hRoot,hCur;//��������Ŀ���
					TV_INSERTSTRUCT TCItem;//�������������ݽṹ
					TCItem.hParent					= TVI_ROOT;//���Ӹ���
					TCItem.hInsertAfter				= TVI_LAST;//�������֮��
					TCItem.item.mask				= TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE;//������
					TCItem.item.pszText				= pNewItem->name;
					TCItem.item.state				= 1;
					TCItem.item.lParam				= 0;//(LPARAM)pNewItem; //���
					if(pNewItem->status == 0)
					{
						TCItem.item.iImage			= 1; 
					}
					else
					{
						TCItem.item.iImage			= 0; 
					}

					hRoot = mCatalogList.InsertItem(&TCItem);//���ظ�����
					mCatalogList.SetItemData(hRoot,(DWORD_PTR)pNewItem);
				}
				else
				{
					memcpy(&iter->second,item,sizeof(GB::CGbCatalogItem));
				}
			}
			break;
		case GB::ECLIENT_EVT_RES_INVITE:
			break;
		default:
			break;
	}
	return 0;
}

void CMT_UIDlg::OnBnClickedButton1()
{
	GB::CGbClientParam LocalParam;
	ZeroMemory(&LocalParam,sizeof(GB::CGbClientParam));

	char szText[256] = {0};
	int nText = -1;
	GetDlgItemText(IDC_LOCAL_ADDR,szText,256);
	strcpy(LocalParam.ip,szText);

	nText = GetDlgItemInt(IDC_LOCAL_PORT);
	LocalParam.port = nText;

	GetDlgItemText(IDC_LOCAL_SERVICE_ID,szText,256);
	strcpy(LocalParam.sip_id,szText);

	GetDlgItemText(IDC_LOCAL_DEMON_ID,szText,256);
	strcpy(LocalParam.domain_id,szText);

	GetDlgItemText(IDC_LOCAL_PROXY,szText,256);
	strcpy(LocalParam.user_agent,szText);

	nText = GetDlgItemInt(IDC_SESS_TIME);
	LocalParam.session_linger_time = nText;

	GB::CGbRegParam ServParam;
	ZeroMemory(&ServParam,sizeof(GB::CGbRegParam));

	GetDlgItemText(IDC_GB_ADDR,szText,256);
	ServParam.server_ip = ntohl(inet_addr(szText));

	GetDlgItemText(IDC_SERVICE_ID,szText,256);
	strcpy(ServParam.server_sip_id,szText);

	GetDlgItemText(IDC_GB_DEMON_ID,szText,256);
	strcpy(ServParam.server_domain_id,szText);

	GetDlgItemText(IDC_SERVICE_PWD,szText,256);
	strcpy(ServParam.passwd,szText);

	nText = GetDlgItemInt(IDC_GB_PORT);
	ServParam.server_port = nText;

	nText = GetDlgItemInt(IDC_INTERVAL);
	ServParam.keep_interval = nText;

	nText = GetDlgItemInt(IDC_MAXCNT);
	ServParam.keep_max_cnt = nText;

	// ����ͻ��˽ӿ�
	int nRet = GB::gb_client_create(&gb_client_handle, LocalParam);
	if (nRet < 0)
	{
		MessageBox("�ͻ��˲�����ʼ��ʧ�ܣ�");
		return;
	}

	nRet = GB::gb_client_set_cb(gb_client_handle, this, GBMessageHandler);
	if (nRet < 0)
	{
		MessageBox("�ͻ������ûص�����ʧ�ܣ�");
		return;
	}

	gb_client_adp_state = ECLIENT_ADP_STATE_REGISTING;
	nRet = GB::gb_client_reg_to_server(gb_client_handle, ServParam);
	if (0 != nRet)
	{
		gb_client_adp_state = ECLIENT_ADP_STATE_INIT;
		MessageBox("ע�����ʧ�ܣ�");
		return;
	}

}


void CMT_UIDlg::OnClose()
{
	for(std::map<std::string,GB::CGbCatalogItem*>::iterator it = mMapCatalogItems.begin();it != mMapCatalogItems.end();it ++)
	{
		delete it->second;
	}
	CDialogEx::OnClose();
}

void CMT_UIDlg::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	HTREEITEM treeitem = mCatalogList.GetSelectedItem();
	GB::CGbCatalogItem* item = (GB::CGbCatalogItem*)mCatalogList.GetItemData(treeitem);

	if(item)
	{
		InviteMedia(item->device_id);
	}
}

void CMT_UIDlg::InviteMedia(char* device_id)
{
	PlayParam* getPP = NULL;
	for (int i = 0;i < 9;i ++)
	{
		if (mtPlayParam[i].mpRtpMedia == NULL)
		{
			getPP = &mtPlayParam[i];
			break;
		}
	}
	if (!getPP)
	{
		MessageBox("û�и���Ĵ��ڿ��Բ����ˣ�");
		return;
	}

	uint32_t sscr = rand();

	int nRet = -1;
	CRtpMediaParam param;
	param.unPort = mnBeginRtpPort;
	param.hWndPlay = getPP->mhPlayWnd;

	getPP->mpRtpMedia = new CRtpMedia;
	getPP->mpRtpMedia->SetPlayParam(&param);
	BOOL bSucess = getPP->mpRtpMedia->Start();
	if(bSucess)
	{
		int nRet = GB::gb_client_invite_media(gb_client_handle,device_id,sscr,mnBeginRtpPort,0);
		mnBeginRtpPort ++;
	}
	else
	{
		delete getPP->mpRtpMedia;
		getPP->mpRtpMedia = NULL;
	}
}