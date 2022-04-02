// IoTimerView.cpp : 实现文件
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "IoTimerView.h"
#include "TaskManager.h"

// CIoTimerView

IMPLEMENT_DYNCREATE(CIoTimerView, CFormView)

CIoTimerView::CIoTimerView()
	: CFormView(CIoTimerView::IDD)
{

}

CIoTimerView::~CIoTimerView()
{
}

void CIoTimerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IOTIMERTABLE, m_IoTimerTable);
}

BEGIN_MESSAGE_MAP(CIoTimerView, CFormView)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_IOTIMERTABLE, &CIoTimerView::OnNMRClickIotimertable)
	ON_UPDATE_COMMAND_UI(ID_IOTIMERMENU_REFRESH, &CIoTimerView::OnUpdateUI)
	ON_COMMAND(ID_IOTIMERMENU_REFRESH, &CIoTimerView::OnRefresh)
	ON_COMMAND(ID_IOTIMERMENU_START, &CIoTimerView::OnStart)
	ON_COMMAND(ID_IOTIMERMENU_STOP, &CIoTimerView::OnStop)
END_MESSAGE_MAP()


// CIoTimerView 诊断

#ifdef _DEBUG
void CIoTimerView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CIoTimerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

CIoTimerDoc* CIoTimerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CIoTimerDoc)));
	return (CIoTimerDoc*)m_pDocument;
}

// CIoTimerView 消息处理程序

void CIoTimerView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_IOTIMER_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_IOTIMER_MENU);
	DWORD ExStyle = m_IoTimerTable.GetExtendedStyle();
	m_IoTimerTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_IoTimerTable.InsertColumn(0,_T("IoTimer对象"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_IoTimerTable.InsertColumn(1,_T("函数地址"),LVCFMT_LEFT);
	m_IoTimerTable.InsertColumn(2,_T("状态"),LVCFMT_LEFT);
	m_IoTimerTable.InsertColumn(3,_T("所在模块"),LVCFMT_LEFT);
	m_IoTimerTable.SetColumnWidth(0, 100);
	m_IoTimerTable.SetColumnWidth(1, 100);
	m_IoTimerTable.SetColumnWidth(2, 100);
	m_IoTimerTable.SetColumnWidth(3, 300);
	InitTable();
}

void CIoTimerView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if(m_IoTimerTable.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		m_IoTimerTable.MoveWindow(&rect);
	}
}

void CIoTimerView::InitTable()
{
	m_IoTimerTable.DeleteAllItems();
	CIoTimerDoc * pitd = GetDocument();
	IoTimerInfoVector * pitiv = &pitd->GetIoTimerInfoVector();
	DWORD size = pitiv->size();
	CString tmp;
	for(DWORD i = 0;i < size;i++)
	{
		PIoTimerInfo piti = &(*pitiv)[i];
		tmp.Format(L"%p",piti->DeviceObject);
		m_IoTimerTable.InsertItem(i,tmp);
		tmp.Format(L"%p",piti->IoTimerRoutineAddress);
		m_IoTimerTable.SetItemText(i,1,tmp);
		tmp.Format(L"%d",piti->ulStatus);
		m_IoTimerTable.SetItemText(i,2,tmp);
		m_IoTimerTable.SetItemText(i,3,piti->FullDllName);
		m_IoTimerTable.SetItemData(i,(DWORD_PTR)piti);
	}
}

void CIoTimerView::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_IOTIMER_MENU, point.x, point.y, this, TRUE);
}

void CIoTimerView::OnNMRClickIotimertable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CIoTimerView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}

void CIoTimerView::OnRefresh()
{
	CTaskManager::GetInstance()->CreateTaskUI(L"DoIoTimerList",CRConsole::DoIoTimerList,GetDocument());
	InitTable();
}

void CIoTimerView::OnStart()
{
	POSITION pos = m_IoTimerTable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = m_IoTimerTable.GetNextSelectedItem(pos);
		PIoTimerInfo piti = (PIoTimerInfo)m_IoTimerTable.GetItemData(iItem);
		if(piti)
		{
			CTaskManager::GetInstance()->CreateTaskUI(L"DoStartIoTimer",CRConsole::DoStartIoTimer,(LPVOID)piti->DeviceObject);
		}
	}
	OnRefresh();
}

void CIoTimerView::OnStop()
{
	POSITION pos = m_IoTimerTable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = m_IoTimerTable.GetNextSelectedItem(pos);
		PIoTimerInfo piti = (PIoTimerInfo)m_IoTimerTable.GetItemData(iItem);
		if(piti)
		{
			CTaskManager::GetInstance()->CreateTaskUI(L"DoStopIoTimer",CRConsole::DoStopIoTimer,(LPVOID)piti->DeviceObject);
			
		}
	}
	OnRefresh();
}
