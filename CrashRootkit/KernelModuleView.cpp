// KernelModuleView.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "KernelModuleView.h"
#include "Transfer.h"
#include "TaskManager.h"


// CKernelModuleView

IMPLEMENT_DYNCREATE(CKernelModuleView, CFormView)

CKernelModuleView::CKernelModuleView()
	: CFormView(CKernelModuleView::IDD)
{

}

CKernelModuleView::~CKernelModuleView()
{
}

void CKernelModuleView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KERNELMODULETABLE, m_cKernelmoduleTable);
}

BEGIN_MESSAGE_MAP(CKernelModuleView, CFormView)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_KERNELMODULETABLE, &CKernelModuleView::OnNMRClickKernelmoduletable)
	ON_UPDATE_COMMAND_UI(ID_KERNELMODULEMENU_REFRESH, &CKernelModuleView::OnUpdateUI)
	ON_COMMAND(ID_KERNELMODULEMENU_REFRESH, &CKernelModuleView::OnRefresh)
END_MESSAGE_MAP()


// CKernelModuleView ���

#ifdef _DEBUG
void CKernelModuleView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CKernelModuleView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CKernelModuleView ��Ϣ�������

void CKernelModuleView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if(m_cKernelmoduleTable.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		m_cKernelmoduleTable.MoveWindow(&rect);
	}
	// TODO: �ڴ˴������Ϣ����������
}

CKernelModuleDoc* CKernelModuleView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CKernelModuleDoc)));
	return (CKernelModuleDoc*)m_pDocument;
}

void CKernelModuleView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_KERNELMODULE_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_KERNELMODULE_MENU);
	DWORD ExStyle = m_cKernelmoduleTable.GetExtendedStyle();
	m_cKernelmoduleTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//������������ѡ�� | ��ʾ����
	m_cKernelmoduleTable.InsertColumn(0,_T("ģ����"),LVCFMT_IMAGE|LVCFMT_LEFT); //��ӵ�0��,�����ƶ�������ʾͼ�������
	m_cKernelmoduleTable.InsertColumn(1,_T("ģ���ַ"),LVCFMT_LEFT);
	m_cKernelmoduleTable.InsertColumn(2,_T("ģ�����"),LVCFMT_LEFT);
	m_cKernelmoduleTable.InsertColumn(3,_T("ģ���С"),LVCFMT_LEFT);
	m_cKernelmoduleTable.SetColumnWidth(0, 300);
	m_cKernelmoduleTable.SetColumnWidth(1, 100);
	m_cKernelmoduleTable.SetColumnWidth(2, 100);
	m_cKernelmoduleTable.SetColumnWidth(3, 100);
	InitTable();
}

void CKernelModuleView::InitTable()
{
	m_cKernelmoduleTable.DeleteAllItems();
	ModuleInfoVector* pmiv = &GetDocument()->GetModuleInfoVector();
	SIZE_T size = pmiv->size();
	for(SIZE_T i=0;i<size;i++)
	{
		PModuleInfo pmi = &(*pmiv)[i];
		CString tmp;
		tmp.Format(L"%s",pmi->FullDllName);
		m_cKernelmoduleTable.InsertItem(i,tmp);
		tmp.Format(L"%p",pmi->BaseAddress);
		m_cKernelmoduleTable.SetItemText(i,1,tmp);
		tmp.Format(L"%p",pmi->EntryPoint);
		m_cKernelmoduleTable.SetItemText(i,2,tmp);
		tmp.Format(L"%X",pmi->SizeOfImage);
		m_cKernelmoduleTable.SetItemText(i,3,tmp);
	}
}

void CKernelModuleView::OnNMRClickKernelmoduletable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CKernelModuleView::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_KERNELMODULE_MENU, point.x, point.y, this, TRUE);
}

void CKernelModuleView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}

void CKernelModuleView::OnRefresh()
{
	CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelModulesList",CRConsole::DoKernelModulesList,GetDocument());
	InitTable();
}
