// SplitterFrame.cpp : 实现文件
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "SplitterFrame.h"
#include "CrashRootkitView.h"
#include "ScriptEditView.h"
#include "OutputView.h"


// CSplitterFrame

IMPLEMENT_DYNCREATE(CSplitterFrame, CMDIChildWndEx)

CSplitterFrame::CSplitterFrame()
{
	m_bIsCreate = FALSE;
}

CSplitterFrame::~CSplitterFrame()
{
}

BOOL CSplitterFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	if(!m_wndSplitter.CreateStatic(this, 2, 1))
		return FALSE;

	CRect cr;
	GetClientRect(&cr);

	if(!m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CScriptEditView),CSize(cr.Width(),cr.Height()),pContext))
		return FALSE;

	if(!m_wndSplitter.CreateView(1,0,RUNTIME_CLASS(COutputView),CSize(cr.Width(),300),pContext))
		return FALSE;

	m_bIsCreate = TRUE;

	return TRUE;
}

BEGIN_MESSAGE_MAP(CSplitterFrame, CMDIChildWndEx)
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CSplitterFrame 消息处理程序

void CSplitterFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWndEx::OnSize(nType, cx, cy);
}

int CSplitterFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}
