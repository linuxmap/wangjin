
// MT_UI.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMT_UIApp:
// �йش����ʵ�֣������ MT_UI.cpp
//

class CMT_UIApp : public CWinApp
{
public:
	CMT_UIApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMT_UIApp theApp;