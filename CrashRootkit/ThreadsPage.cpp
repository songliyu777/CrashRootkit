// ThreadsPage.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "ThreadsPage.h"
#include "ProcessDoc.h"
#include "ProcessView.h"
#include "TaskManager.h"


// CThreadsPage �Ի���

IMPLEMENT_DYNAMIC(CThreadsPage, CDialog)

CThreadsPage::CThreadsPage(CWnd* pParent /*=NULL*/)
	: CDialog(CThreadsPage::IDD, pParent)
{

}

CThreadsPage::~CThreadsPage()
{
}

void CThreadsPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_THREADSTABLE, m_cThreadsTable);
}


BEGIN_MESSAGE_MAP(CThreadsPage, CDialog)
	ON_MESSAGE(WM_DOUPDATEDATA,DoUpdateData)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_THREADSTABLE, &CThreadsPage::OnNMRClickThreadstable)
	ON_COMMAND(ID_THREAD_REFRESH, &CThreadsPage::OnRefresh)
	ON_COMMAND(ID_THREAD_TERMINATE,&CThreadsPage::OnTerminate)
END_MESSAGE_MAP()


// CThreadsPage ��Ϣ�������

BOOL CThreadsPage::OnInitDialog()
{
	CDialog::OnInitDialog();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_THREAD_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_THREAD_MENU);
	DWORD ExStyle = m_cThreadsTable.GetExtendedStyle();
	m_cThreadsTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//������������ѡ�� | ��ʾ����
	m_cThreadsTable.InsertColumn(0,_T("�߳�ID"),LVCFMT_IMAGE|LVCFMT_LEFT); //��ӵ�0��,�����ƶ�������ʾͼ�������
	m_cThreadsTable.InsertColumn(1,_T("TEB��ַ"),LVCFMT_LEFT);
	m_cThreadsTable.InsertColumn(2,_T("�߳����"),LVCFMT_LEFT);
	m_cThreadsTable.InsertColumn(3,_T("�߳�״̬"),LVCFMT_LEFT);
	m_cThreadsTable.InsertColumn(4,_T("�ں�ʱ��"),LVCFMT_LEFT);
	m_cThreadsTable.InsertColumn(5,_T("�û�ʱ��"),LVCFMT_LEFT);
	m_cThreadsTable.InsertColumn(6,_T("����ģ��"),LVCFMT_LEFT);
	m_cThreadsTable.SetColumnWidth(0, 100);
	m_cThreadsTable.SetColumnWidth(1, 100);
	m_cThreadsTable.SetColumnWidth(2, 100);
	m_cThreadsTable.SetColumnWidth(3, 100);
	m_cThreadsTable.SetColumnWidth(4, 100);
	m_cThreadsTable.SetColumnWidth(5, 100);
	m_cThreadsTable.SetColumnWidth(6, 400);
	return TRUE;
}

void CThreadsPage::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	CRect rc;
	CWnd* parent = GetParent();
	parent->GetClientRect(rc);
	rc.top += 25;
	rc.left += 5;
	rc.right -= 5;
	rc.bottom -= 5;
	MoveWindow(&rc);

	if(m_cThreadsTable.GetSafeHwnd())
	{
		GetClientRect(&rc);
		m_cThreadsTable.MoveWindow(&rc);
	}
}

LRESULT CThreadsPage::DoUpdateData(WPARAM wParam, LPARAM lParam)
{
	if(wParam && lParam)
	{
		PProcessInfo ppi = (PProcessInfo)wParam;
		CProcessDoc *ppd = (CProcessDoc *)lParam;
		ppd->m_pCurrentProcessInfo = ppi;
		CTaskManager::GetInstance()->CreateTaskUI(L"DoThreadsList",CRConsole::DoThreadsList,ppd);
		InitTable(&ppd->GetThreadInfoVector());
	}
	return 0;
}

void CThreadsPage::InitTable(ThreadInfoVector *ptiv)
{
	m_cThreadsTable.DeleteAllItems();
	SIZE_T size = ptiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PThreadInfo pti = &(*ptiv)[i];
		CString tmp;
		tmp.Format(L"%d",pti->UniqueThread);
		m_cThreadsTable.InsertItem(i,tmp);
		tmp.Format(L"%p",pti->Teb);
		m_cThreadsTable.SetItemText(i,1,tmp);
		if(pti->Win32StartAddress)
		{
			tmp.Format(L"%p",pti->Win32StartAddress);
		}
		else if(pti->StartAddress)
		{
			tmp.Format(L"%p",pti->StartAddress);
		}
		m_cThreadsTable.SetItemText(i,2,tmp);
		tmp.Format(L"%d",pti->State);
		m_cThreadsTable.SetItemText(i,3,tmp);
		tmp.Format(L"%d",pti->KernelTime);
		m_cThreadsTable.SetItemText(i,4,tmp);
		tmp.Format(L"%d",pti->UserTime);
		m_cThreadsTable.SetItemText(i,5,tmp);
		tmp.Format(L"%s",pti->FullDllName);
		m_cThreadsTable.SetItemText(i,6,tmp);
		m_cThreadsTable.SetItemData(i,(DWORD_PTR)pti);
	}
}

void CThreadsPage::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_THREAD_MENU, point.x, point.y, this, TRUE);
}

void CThreadsPage::OnNMRClickThreadstable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CThreadsPage::OnRefresh()
{
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	CTaskManager::GetInstance()->CreateTaskUI(L"DoThreadsList",CRConsole::DoThreadsList,ppd);
	InitTable(&ppd->GetThreadInfoVector());
}

void CThreadsPage::OnTerminate()
{
	POSITION pos = m_cThreadsTable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = m_cThreadsTable.GetNextSelectedItem(pos);
		PThreadInfo pti = (PThreadInfo)m_cThreadsTable.GetItemData(iItem);
		if(pti)
		{
			CTaskManager::GetInstance()->CreateTaskUI(L"DoTerminateThread",CRConsole::DoTerminateThread,pti);
		}
	}
	OnRefresh();
}