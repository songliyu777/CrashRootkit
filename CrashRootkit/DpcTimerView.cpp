// DpcTimerView.cpp : 实现文件
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "DpcTimerView.h"
#include "TaskManager.h"


// CDpcTimerView

IMPLEMENT_DYNCREATE(CDpcTimerView, CFormView)

CDpcTimerView::CDpcTimerView()
	: CFormView(CDpcTimerView::IDD)
{

}

CDpcTimerView::~CDpcTimerView()
{
}

void CDpcTimerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DPCTIMERTABLE, m_cDpcTimerTable);
}

BEGIN_MESSAGE_MAP(CDpcTimerView, CFormView)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_DPCTIMERTABLE, &CDpcTimerView::OnNMRClickDpctimertable)
	ON_UPDATE_COMMAND_UI(ID_DPCTIMERMENU_REFRESH, &CDpcTimerView::OnUpdateUI)
	ON_COMMAND(ID_DPCTIMERMENU_REFRESH, &CDpcTimerView::OnRefresh)
	ON_COMMAND(ID_DPCTIMERMENU_KILL, &CDpcTimerView::OnKillDpcTimer)
END_MESSAGE_MAP()


// CDpcTimerView 诊断

#ifdef _DEBUG
void CDpcTimerView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CDpcTimerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


CDpcTimerDoc* CDpcTimerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDpcTimerDoc)));
	return (CDpcTimerDoc*)m_pDocument;
}

// CDpcTimerView 消息处理程序

void CDpcTimerView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if(m_cDpcTimerTable.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		m_cDpcTimerTable.MoveWindow(&rect);
	}
}

void CDpcTimerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_DPCTIMER_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_DPCTIMER_MENU);
	DWORD ExStyle = m_cDpcTimerTable.GetExtendedStyle();
	m_cDpcTimerTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_cDpcTimerTable.InsertColumn(0,_T("KTimer地址"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_cDpcTimerTable.InsertColumn(1,_T("Dpc地址"),LVCFMT_LEFT);
	m_cDpcTimerTable.InsertColumn(2,_T("函数地址"),LVCFMT_LEFT);
	m_cDpcTimerTable.InsertColumn(3,_T("间隔时间"),LVCFMT_LEFT);
	m_cDpcTimerTable.InsertColumn(4,_T("所在模块"),LVCFMT_LEFT);
	m_cDpcTimerTable.SetColumnWidth(0, 100);
	m_cDpcTimerTable.SetColumnWidth(1, 100);
	m_cDpcTimerTable.SetColumnWidth(2, 100);
	m_cDpcTimerTable.SetColumnWidth(3, 100);
	m_cDpcTimerTable.SetColumnWidth(4, 300);
	InitTable();
}

void CDpcTimerView::InitTable()
{
	m_cDpcTimerTable.DeleteAllItems();
	CDpcTimerDoc * pdtd = GetDocument();
	DpcTimerInfoVector * pdtiv = &pdtd->GetDpcTimerInfoVector();
	DWORD size = pdtiv->size();
	CString tmp;
	for(DWORD i = 0;i < size;i++)
	{
		PDpcTimerInfo pdti = &(*pdtiv)[i];
		tmp.Format(L"%p",pdti->TimerAddress);
		m_cDpcTimerTable.InsertItem(i,tmp);
		tmp.Format(L"%p",pdti->DpcAddress);
		m_cDpcTimerTable.SetItemText(i,1,tmp);
		tmp.Format(L"%p",pdti->DpcRoutineAddress);
		m_cDpcTimerTable.SetItemText(i,2,tmp);
		tmp.Format(L"%d",pdti->Period);
		m_cDpcTimerTable.SetItemText(i,3,tmp);
		m_cDpcTimerTable.SetItemText(i,4,pdti->FullDllName);
		m_cDpcTimerTable.SetItemData(i,(DWORD_PTR)pdti);
	}
}

void CDpcTimerView::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_DPCTIMER_MENU, point.x, point.y, this, TRUE);
}

void CDpcTimerView::OnNMRClickDpctimertable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CDpcTimerView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}

void CDpcTimerView::OnRefresh()
{
	CTaskManager::GetInstance()->CreateTaskUI(L"DoDpcTimerList",CRConsole::DoDpcTimerList,GetDocument());
	InitTable();
}

void CDpcTimerView::OnKillDpcTimer()
{
	POSITION pos = m_cDpcTimerTable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = m_cDpcTimerTable.GetNextSelectedItem(pos);
		PDpcTimerInfo pdti = (PDpcTimerInfo)m_cDpcTimerTable.GetItemData(iItem);
		if(pdti)
		{
			CTaskManager::GetInstance()->CreateTaskUI(L"DoKillDpcTimer",CRConsole::DoKillDpcTimer,(LPVOID)pdti->TimerAddress);
		}
	}
	OnRefresh();
}