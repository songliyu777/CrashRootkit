#include "StdAfx.h"
#include "CrashRootkit.h"
#include "KernelHookView.h"
#include "KernelHookDoc.h"
#include "ConsoleDll.h"
#include "Transfer.h"
#include "TaskManager.h"

IMPLEMENT_DYNCREATE(CKernelHookView, CFormView)

BEGIN_MESSAGE_MAP(CKernelHookView, CFormView)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_KERNELHOOKTABLE, &CKernelHookView::OnNMRClickKernelhooktable)
	ON_UPDATE_COMMAND_UI(ID_KERNELHOOK_REFRESH, &CKernelHookView::OnUpdateUI)
	ON_COMMAND(ID_KERNELHOOK_REFRESH, &CKernelHookView::OnRefresh)
	ON_COMMAND(ID_KERNELHOOK_RECOVER, &CKernelHookView::OnRecover)
END_MESSAGE_MAP()

CKernelHookView::CKernelHookView(void)
	: CFormView(CKernelHookView::IDD)
{
}

CKernelHookView::~CKernelHookView(void)
{
}

void CKernelHookView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KERNELHOOKTABLE, n_cKernelHookTable);
}

void CKernelHookView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_KERNELHOOK_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_KERNELHOOK_MENU);
	DWORD ExStyle = n_cKernelHookTable.GetExtendedStyle();
	n_cKernelHookTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	n_cKernelHookTable.InsertColumn(0,_T("地址"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	n_cKernelHookTable.InsertColumn(1,_T("长度"),LVCFMT_LEFT);
	n_cKernelHookTable.InsertColumn(2,_T("所在模块"),LVCFMT_LEFT);
	n_cKernelHookTable.InsertColumn(3,_T("类型"),LVCFMT_LEFT);
	n_cKernelHookTable.InsertColumn(4,_T("跳转位置"),LVCFMT_LEFT);
	n_cKernelHookTable.InsertColumn(5,_T("原始字节"),LVCFMT_LEFT);
	n_cKernelHookTable.InsertColumn(6,_T("当前字节"),LVCFMT_LEFT);
	n_cKernelHookTable.SetColumnWidth(0, 100);
	n_cKernelHookTable.SetColumnWidth(1, 50);
	n_cKernelHookTable.SetColumnWidth(2, 100);
	n_cKernelHookTable.SetColumnWidth(3, 50);
	n_cKernelHookTable.SetColumnWidth(4, 250);
	n_cKernelHookTable.SetColumnWidth(5, 250);
	n_cKernelHookTable.SetColumnWidth(6, 250);
	InitTable();
}

void CKernelHookView::InitTable()
{
	n_cKernelHookTable.DeleteAllItems();
	MemoryHookVector * pmhv = &GetDocument()->GetMemoryHookVector();
	DWORD size = pmhv->size();
	MemoryHookVector::iterator pos;
	CString tmp,tmpc,tmpo;
	WCHAR szImageName[MAX_PATH];
	for(DWORD i = 0;i < size;i++)
	{
		PMemoryHook pmh = &(*pmhv)[i];
		tmp.Format(L"%p",pmh->Address);
		n_cKernelHookTable.InsertItem(i,tmp);
		tmp.Format(L"%d",pmh->Length);
		n_cKernelHookTable.SetItemText(i,1,tmp);
		GetImageNameByPath(szImageName,pmh->ModuleName);
		n_cKernelHookTable.SetItemText(i,2,szImageName);
		switch(pmh->Type)
		{
		case IAT_HOOK:
			tmp.Format(L"IAT");
			break;
		case EAT_HOOK:
			tmp.Format(L"EAT");
			break;
		case INLINE_HOOK:
			tmp.Format(L"INLINE");
			break;
		case SSDT_HOOK:
			tmp.Format(L"SSDT");
			break;
		case UNKNOW_HOOK:
			tmp.Format(L"UNKNOW");
			break;
		}
		n_cKernelHookTable.SetItemText(i,3,tmp);
		tmpc.Format(L"");
		tmpo.Format(L"");
		for(DWORD j = 0;j < pmh->Length;j++)
		{
			tmp.Format(L"%02X ",pmh->Origin[j]);
			tmpo += tmp;
			tmp.Format(L"%02X ",pmh->Current[j]);
			tmpc += tmp;
		}
		n_cKernelHookTable.SetItemText(i,4,pmh->JmpModuleName);
		n_cKernelHookTable.SetItemText(i,5,tmpo);
		n_cKernelHookTable.SetItemText(i,6,tmpc);
		n_cKernelHookTable.SetItemData(i,(DWORD_PTR)pmh);
	}
}

#ifdef _DEBUG
void CKernelHookView::AssertValid() const
{
	CFormView::AssertValid();
}

void CKernelHookView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CKernelHookDoc* CKernelHookView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CKernelHookDoc)));
	return (CKernelHookDoc*)m_pDocument;
}
#endif //_DEBUG

void CKernelHookView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if(n_cKernelHookTable.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		n_cKernelHookTable.MoveWindow(&rect);
	}
}

void CKernelHookView::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_KERNELHOOK_MENU, point.x, point.y, this, TRUE);
}

void CKernelHookView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}

void CKernelHookView::OnRefresh()
{
	CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelHookCheck",CRConsole::DoKernelHookCheck,GetDocument());
	InitTable();
}

void CKernelHookView::OnNMRClickKernelhooktable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CKernelHookView::OnRecover()
{
	POSITION pos = n_cKernelHookTable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = n_cKernelHookTable.GetNextSelectedItem(pos);
		PMemoryHook pmh = (PMemoryHook)n_cKernelHookTable.GetItemData(iItem);
		if(pmh)
		{
			CTaskManager::GetInstance()->CreateTaskUI(L"DoKernelHookRecover",CRConsole::DoKernelHookRecover,pmh);
		}
	}
	OnRefresh();
}
