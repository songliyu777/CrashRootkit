#include "StdAfx.h"
#include "CrashRootkit.h"
#include "ProcessView.h"
#include "ProcessDoc.h"
#include "Transfer.h"
#include "ModulesPage.h"
#include "ThreadsPage.h"
#include "HookCheckPage.h"
#include "ConsoleDll.h"
#include "TaskManager.h"


IMPLEMENT_DYNCREATE(CProcessView, CFormView)

BEGIN_MESSAGE_MAP(CProcessView, CFormView)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_PROCESSTABLE, &CProcessView::OnNMClickProcesstable)
	ON_NOTIFY(NM_RCLICK, IDC_PROCESSTABLE, &CProcessView::OnNMRClickProcesstable)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PROCESSTAB, &CProcessView::OnTcnSelchangeProcesstab)
	ON_UPDATE_COMMAND_UI(IDR_PROCCESS_MENU_REFRESH, &CProcessView::OnUpdateUI)
	ON_COMMAND(IDR_PROCCESS_MENU_REFRESH, &CProcessView::OnRefresh)
	ON_COMMAND(ID_PROCESSE_MENU_TERMINATE, &CProcessView::OnTerminate)
END_MESSAGE_MAP()

CProcessView::CProcessView(void)
	: CFormView(CProcessView::IDD)
{
}

CProcessView::~CProcessView(void)
{
	POSITION pos = m_PageList.GetHeadPosition();
	while(pos != NULL)   
	{   
		CDialog* pPage = m_PageList.GetNext(pos);
		delete pPage;
	}
	m_PageList.RemoveAll();
}

void CProcessView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESSTABLE, m_cProcessTable);
	DDX_Control(pDX, IDC_PROCESSTAB, m_cTabCtrl);
}

void CProcessView::InitTable()
{
	m_cProcessTable.DeleteAllItems();
	ProcessInfoVector* piv = &GetDocument()->GetProcessInfoVector();
	SIZE_T size = piv->size();
	CString tmp;
	WCHAR szImageName[MAX_PATH];
	for(SIZE_T i=0;i<size;i++)
	{
		PProcessInfo ppi = &(*piv)[i];
		if(ppi->ProcessId == 4)
		{
			m_cProcessTable.InsertItem(i,L"System");
		}
		else
		{
			if(wcslen(ppi->ImagePath))
			{
				GetImageNameByPath(szImageName,ppi->ImagePath);
				m_cProcessTable.InsertItem(i,szImageName);
			}
			else if(strlen((const char *)ppi->ImageFileName))
			{
				CString ifn;
				ifn.Format(L"%S",ppi->ImageFileName);
				m_cProcessTable.InsertItem(i,ifn);
			}
		}
		tmp.Format(L"%d",ppi->ProcessId);
		m_cProcessTable.SetItemText(i,1,tmp);
		tmp.Format(L"%d",ppi->ParentProcessId);
		m_cProcessTable.SetItemText(i,2,tmp);
		tmp.Format(L"%p",ppi->PEProcess);
		m_cProcessTable.SetItemText(i,3,tmp);
		tmp.Format(L"%s",ppi->ImagePath);
		m_cProcessTable.SetItemText(i,4,tmp);
		m_cProcessTable.SetItemData(i,(DWORD_PTR)ppi);
	}
}

void CProcessView::CreatePage(int page)
{
	if(m_cTabCtrl.GetSafeHwnd())
	{
		TCITEMW it;
		it.mask = TCIF_TEXT;

		switch(page)
		{
		case 0:
			{
				it.pszText=_T("Modules");
				m_cTabCtrl.InsertItem(page,&it);
				CDialog* pPage = new CModulesPage();
				pPage->Create(IDD_MODULESPAGE,&m_cTabCtrl);
				m_PageList.AddTail(pPage);
				pPage->ShowWindow(SW_SHOW);
			}
			break;
		case 1:
			{
				it.pszText=_T("Threads");
				m_cTabCtrl.InsertItem(page,&it);
				CDialog* pPage = new CThreadsPage();
				pPage->Create(IDD_THREADSPAGE,&m_cTabCtrl);
				m_PageList.AddTail(pPage);
				pPage->ShowWindow(SW_HIDE);
			}
			break;
		case 2:
			{
				it.pszText=_T("Hook Check");
				m_cTabCtrl.InsertItem(page,&it);
				CDialog* pPage = new CHookCheckPage();
				pPage->Create(IDD_HOOKCHECKPAGE,&m_cTabCtrl);
				m_PageList.AddTail(pPage);
				pPage->ShowWindow(SW_HIDE);
			}
			break;
		}
	}
}

void CProcessView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_PROCESS_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_PROCESS_MENU);
	DWORD ExStyle = m_cProcessTable.GetExtendedStyle();
	m_cProcessTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_cProcessTable.InsertColumn(0,_T("镜像名"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_cProcessTable.InsertColumn(1,_T("进程ID"),LVCFMT_LEFT);
	m_cProcessTable.InsertColumn(2,_T("父进程ID"),LVCFMT_LEFT);
	m_cProcessTable.InsertColumn(3,_T("EPROCESS"),LVCFMT_LEFT);
	m_cProcessTable.InsertColumn(4,_T("进程路径"),LVCFMT_LEFT);
	m_cProcessTable.SetColumnWidth(0, 150);
	m_cProcessTable.SetColumnWidth(1, 100);
	m_cProcessTable.SetColumnWidth(2, 100);
	m_cProcessTable.SetColumnWidth(3, 100);
	m_cProcessTable.SetColumnWidth(4, 350);
	InitTable();
	CreatePage(0);
	CreatePage(1);
	CreatePage(2);
}

#ifdef _DEBUG
void CProcessView::AssertValid() const
{
	CFormView::AssertValid();
}

void CProcessView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CProcessDoc* CProcessView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CProcessDoc)));
	return (CProcessDoc*)m_pDocument;
}
#endif //_DEBUG

void CProcessView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if(m_cProcessTable.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		rect.bottom = rect.Height()/2;
		m_cProcessTable.MoveWindow(&rect);
	}

	if(m_cTabCtrl.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top = rect.Height()/2;
		m_cTabCtrl.MoveWindow(&rect);
		POSITION pos = m_PageList.GetHeadPosition();
		while(pos != NULL)   
		{   
			CDialog* pPage = m_PageList.GetNext(pos);
			pPage->MoveWindow(&rect);
		}   
	}
}

void CProcessView::OnNMClickProcesstable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	int nItem = pNMItemActivate->iItem;
	if(nItem==-1)
		return;
	DWORD_PTR dptr = m_cProcessTable.GetItemData(nItem);
	POSITION pos = m_PageList.GetHeadPosition();
	while(pos != NULL)   
	{   
		CDialog* pPage = m_PageList.GetNext(pos);
		pPage->SendMessage(WM_DOUPDATEDATA,(WPARAM)dptr,(LPARAM)GetDocument());
	}   
	*pResult = 0;
}

void CProcessView::OnNMRClickProcesstable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CProcessView::OnTcnSelchangeProcesstab(NMHDR *pNMHDR, LRESULT *pResult)
{
	POSITION pos = m_PageList.GetHeadPosition();
	while(pos != NULL)   
	{   
		CDialog* pPage = m_PageList.GetNext(pos);
		pPage->ShowWindow(SW_HIDE);
	}
	int index = m_cTabCtrl.GetCurSel();
	if(index==-1)return;
	pos = m_PageList.FindIndex(index);
	CDialog* pPage = m_PageList.GetAt(pos);
	pPage->ShowWindow(SW_SHOW);
	*pResult = 0;
	*pResult = 0;
}

void CProcessView::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_PROCESS_MENU, point.x, point.y, this, TRUE);
}

void CProcessView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}

void CProcessView::OnRefresh()
{
	CTaskManager::GetInstance()->CreateTaskUI(L"DoProcessesList",CRConsole::DoProcessesList,GetDocument());
	InitTable();
}

void CProcessView::OnTerminate()
{
	POSITION pos = m_cProcessTable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = m_cProcessTable.GetNextSelectedItem(pos);
		PProcessInfo ppi = (PProcessInfo)m_cProcessTable.GetItemData(iItem);
		if(ppi)
		{
			CTaskManager::GetInstance()->CreateTaskUI(L"DoTerminateProcess",CRConsole::DoTerminateProcess,(LPVOID)ppi->ProcessId);
			OnRefresh();
		}
	}
}