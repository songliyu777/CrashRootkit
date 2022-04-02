// HookCheckPage.cpp : 实现文件
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "HookCheckPage.h"
#include "ProcessDoc.h"
#include "ProcessView.h"
#include "TaskManager.h"


// CHookCheckPage 对话框

IMPLEMENT_DYNAMIC(CHookCheckPage, CDialog)

CHookCheckPage::CHookCheckPage(CWnd* pParent /*=NULL*/)
	: CDialog(CHookCheckPage::IDD, pParent)
{

}

CHookCheckPage::~CHookCheckPage()
{
}

void CHookCheckPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOOKCHECKTABLE, m_hookchecktable);
}

BEGIN_MESSAGE_MAP(CHookCheckPage, CDialog)
	ON_MESSAGE(WM_DOUPDATEDATA,DoUpdateData)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_HOOKCHECKTABLE, &CHookCheckPage::OnNMRClickHookchecktable)
	ON_COMMAND(ID_HOOKCHECK_REFRESH, &CHookCheckPage::OnRefresh)
	ON_COMMAND(ID_HOOKCHECK_RECOVER,&CHookCheckPage::OnRecover)
END_MESSAGE_MAP()


BOOL CHookCheckPage::OnInitDialog()
{
	CDialog::OnInitDialog();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_HOOKCHECK_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_HOOKCHECK_MENU);
	DWORD ExStyle = m_hookchecktable.GetExtendedStyle();
	m_hookchecktable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_hookchecktable.InsertColumn(0,_T("地址"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_hookchecktable.InsertColumn(1,_T("长度"),LVCFMT_LEFT);
	m_hookchecktable.InsertColumn(2,_T("所在模块"),LVCFMT_LEFT);
	m_hookchecktable.InsertColumn(3,_T("原始字节"),LVCFMT_LEFT);
	m_hookchecktable.InsertColumn(4,_T("当前字节"),LVCFMT_LEFT);
	m_hookchecktable.SetColumnWidth(0, 100);
	m_hookchecktable.SetColumnWidth(1, 50);
	m_hookchecktable.SetColumnWidth(2, 100);
	m_hookchecktable.SetColumnWidth(3, 250);
	m_hookchecktable.SetColumnWidth(4, 250);
	return TRUE;
}

void CHookCheckPage::InitTable(MemoryHookVector *pmhv)
{
	m_hookchecktable.DeleteAllItems();
	DWORD size = pmhv->size();
	MemoryHookVector::iterator pos;
	CString tmp,tmpc,tmpo;
	for(DWORD i = 0;i < size;i++)
	{
		PMemoryHook pmh = &(*pmhv)[i];
		tmp.Format(L"%p",pmh->Address);
		m_hookchecktable.InsertItem(i,tmp);
		tmp.Format(L"%d",pmh->Length);
		m_hookchecktable.SetItemText(i,1,tmp);
		m_hookchecktable.SetItemText(i,2,pmh->ModuleName);
		tmpc.Format(L"");
		tmpo.Format(L"");
		for(DWORD j = 0;j < pmh->Length;j++)
		{
			tmp.Format(L"%02X ",pmh->Origin[j]);
			tmpo += tmp;
			tmp.Format(L"%02X ",pmh->Current[j]);
			tmpc += tmp;
		}
		m_hookchecktable.SetItemText(i,3,tmpo);
		m_hookchecktable.SetItemText(i,4,tmpc);
		m_hookchecktable.SetItemData(i,(DWORD_PTR)pmh);
	}
}
// CHookCheckPage 消息处理程序

void CHookCheckPage::OnSize(UINT nType, int cx, int cy)
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

	if(m_hookchecktable.GetSafeHwnd())
	{
		GetClientRect(&rc);
		m_hookchecktable.MoveWindow(&rc);
	}
}

LRESULT CHookCheckPage::DoUpdateData(WPARAM wParam, LPARAM lParam)
{
	if(wParam && lParam)
	{
		PProcessInfo ppi = (PProcessInfo)wParam;
		CProcessDoc *ppd = (CProcessDoc *)lParam;
		ppd->m_pCurrentProcessInfo = ppi;
		ppd->ClearMemoryHookVector();
		//CTaskManager::GetInstance()->CreateTaskUI(L"DoHookCheckList",CRConsole::DoHookCheckList,ppd,FALSE);
		InitTable(&ppd->GetMemoryHookVector());
	}
	return 0;
}

void CHookCheckPage::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_HOOKCHECK_MENU, point.x, point.y, this, TRUE);
}

void CHookCheckPage::OnNMRClickHookchecktable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CHookCheckPage::OnRefresh()
{
	CProcessDoc * ppd = (CProcessDoc *)CTaskManager::GetInstance()->GetDocument(RUNTIME_CLASS(CProcessDoc));
	CTaskManager::GetInstance()->CreateTaskUI(L"DoHookCheckList",CRConsole::DoHookCheckList,ppd,FALSE);
	InitTable(&ppd->GetMemoryHookVector());
}

void CHookCheckPage::OnRecover()
{
	POSITION pos = m_hookchecktable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = m_hookchecktable.GetNextSelectedItem(pos);
		PMemoryHook pmh = (PMemoryHook)m_hookchecktable.GetItemData(iItem);
		if(pmh)
		{
			CTaskManager::GetInstance()->CreateTaskUI(L"DoHookCheckRecover",CRConsole::DoHookCheckRecover,pmh);
		}
	}
	OnRefresh();
}