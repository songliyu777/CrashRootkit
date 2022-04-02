// ModulesPage.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "ProcessView.h"
#include "ModulesPage.h"
#include "ConsoleDll.h"
#include "Transfer.h"
#include "TaskManager.h"


// CModulesPage �Ի���

IMPLEMENT_DYNAMIC(CModulesPage, CDialog)

CModulesPage::CModulesPage(CWnd* pParent /*=NULL*/)
	: CDialog(CModulesPage::IDD, pParent)
{

}

CModulesPage::~CModulesPage()
{

}

void CModulesPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODULESTABLE, m_cModulesTable);
}


BEGIN_MESSAGE_MAP(CModulesPage, CDialog)
	ON_MESSAGE(WM_DOUPDATEDATA,DoUpdateData)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_MODULESTABLE, &CModulesPage::OnNMRClickModulestable)
END_MESSAGE_MAP()


// CModulesPage ��Ϣ�������

void CModulesPage::OnSize(UINT nType, int cx, int cy)
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

	if(m_cModulesTable.GetSafeHwnd())
	{
		GetClientRect(&rc);
		m_cModulesTable.MoveWindow(&rc);
	}
}

BOOL CModulesPage::OnInitDialog()
{
	CDialog::OnInitDialog();
	DWORD ExStyle = m_cModulesTable.GetExtendedStyle();
	m_cModulesTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//������������ѡ�� | ��ʾ����
	m_cModulesTable.InsertColumn(0,_T("ģ����"),LVCFMT_IMAGE|LVCFMT_LEFT); //��ӵ�0��,�����ƶ�������ʾͼ�������
	m_cModulesTable.InsertColumn(1,_T("ģ���ַ"),LVCFMT_LEFT);
	m_cModulesTable.InsertColumn(2,_T("ģ�����"),LVCFMT_LEFT);
	m_cModulesTable.InsertColumn(3,_T("ģ���С"),LVCFMT_LEFT);
	m_cModulesTable.SetColumnWidth(0, 300);
	m_cModulesTable.SetColumnWidth(1, 100);
	m_cModulesTable.SetColumnWidth(2, 100);
	m_cModulesTable.SetColumnWidth(3, 100);
	return TRUE;
}

LRESULT CModulesPage::DoUpdateData(WPARAM wParam, LPARAM lParam)
{
	if(wParam && lParam)
	{
		PProcessInfo ppi = (PProcessInfo)wParam;
		CProcessDoc *ppd = (CProcessDoc *)lParam;
		ppd->m_pCurrentProcessInfo = ppi;
		CTaskManager::GetInstance()->CreateTaskUI(L"DoModulesList",CRConsole::DoModulesList,ppd);
		InitTable(&ppd->GetModuleInfoVector());
	}
	return 0;
}

void CModulesPage::InitTable(ModuleInfoVector *pmiv)
{
	m_cModulesTable.DeleteAllItems();
	SIZE_T size = pmiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PModuleInfo pmi = &(*pmiv)[i];
		CString tmp;
		tmp.Format(L"%s",pmi->FullDllName);
		m_cModulesTable.InsertItem(i,tmp);
		tmp.Format(L"%p",pmi->BaseAddress);
		m_cModulesTable.SetItemText(i,1,tmp);
		tmp.Format(L"%p",pmi->EntryPoint);
		m_cModulesTable.SetItemText(i,2,tmp);
		tmp.Format(L"%X",pmi->SizeOfImage);
		m_cModulesTable.SetItemText(i,3,tmp);
	}
}

void CModulesPage::OnNMRClickModulestable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}
