#include "stdafx.h"
#include "SSDTView.h"
#include "SSDTDoc.h"
#include "CrashRootkit.h"
#include "TaskManager.h"
#include "Log.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CSSDTView, CFormView)

BEGIN_MESSAGE_MAP(CSSDTView, CFormView)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SSDTTABLE, &CSSDTView::OnNMCustomdrawSsdttable)
	ON_NOTIFY(NM_RCLICK, IDC_SSDTTABLE, &CSSDTView::OnNMRClickSsdttable)
	ON_UPDATE_COMMAND_UI(ID_SSDTMENU_REFRESH, &CSSDTView::OnUpdateUI)
	ON_COMMAND(ID_SSDTMENU_REFRESH, &CSSDTView::OnRefresh)
	ON_COMMAND(ID_SSDTMENU_ALL, &CSSDTView::OnShowAll)
	ON_COMMAND(ID_SSDTMENU_HOOK, &CSSDTView::OnShowHook)
	ON_COMMAND(ID_SSDTMENU_RECOVER, &CSSDTView::OnRecover)
END_MESSAGE_MAP()

CSSDTView::CSSDTView(void)
	: CFormView(CSSDTView::IDD)
{
}

CSSDTView::~CSSDTView(void)
{
}

void CSSDTView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SSDTTABLE, m_cSSTDTable);
}

void CSSDTView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_SSDT_MENU);
	ASSERT(bNameValid);
	theApp.GetContextMenuManager()->AddMenu(strName, IDR_SSDT_MENU);
	DWORD ExStyle = m_cSSTDTable.GetExtendedStyle();
	m_cSSTDTable.SetExtendedStyle(ExStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);//设置属性整行选择 | 显示网格
	m_cSSTDTable.InsertColumn(0,_T("索引号"),LVCFMT_IMAGE|LVCFMT_LEFT); //添加第0列,可以制定允许显示图标等属性
	m_cSSTDTable.InsertColumn(1,_T("函数名"),LVCFMT_LEFT);
	m_cSSTDTable.InsertColumn(2,_T("当前函数地址"),LVCFMT_CENTER);
	m_cSSTDTable.InsertColumn(3,_T("原函数地址"),LVCFMT_CENTER);
	m_cSSTDTable.InsertColumn(4,_T("模块名称"),LVCFMT_LEFT);
	m_cSSTDTable.SetColumnWidth(0, 100);
	m_cSSTDTable.SetColumnWidth(1, 250);
	m_cSSTDTable.SetColumnWidth(2, 120);
	m_cSSTDTable.SetColumnWidth(3, 120);
	m_cSSTDTable.SetColumnWidth(4, 300);
	InitTable();
}

#ifdef _DEBUG
void CSSDTView::AssertValid() const
{
	CView::AssertValid();
}

void CSSDTView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSSDTDoc* CSSDTView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSSDTDoc)));
	return (CSSDTDoc*)m_pDocument;
}
#endif //_DEBUG

void CSSDTView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if(m_cSSTDTable.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		m_cSSTDTable.MoveWindow(&rect);
	}
}

void CSSDTView::InitTable()
{
	m_cSSTDTable.DeleteAllItems();
	SSDTInfoVector* siv = &GetDocument()->GetSSDTInfoVector();
	BOOL bShowAll = GetDocument()->IsShowAll();
	SIZE_T size = siv->size();
	CString tmp;
	SIZE_T index = 0;
	for(SIZE_T i=0;i<size;i++)
	{
		PSSDTInfo psi = &(*siv)[i];
		if(!bShowAll)
		{
			if(psi->CurrentAddress!=psi->OriginalAddress)
			{
				tmp.Format(L"%d",psi->Index);
				m_cSSTDTable.InsertItem(index,tmp);
				tmp.Format(L"%S",psi->FunctionName);
				m_cSSTDTable.SetItemText(index,1,tmp);
				tmp.Format(L"%p",psi->CurrentAddress);
				m_cSSTDTable.SetItemText(index,2,tmp);
				tmp.Format(L"%p",psi->OriginalAddress);
				m_cSSTDTable.SetItemText(index,3,tmp);
				tmp.Format(L"%s",psi->FullDllName);
				m_cSSTDTable.SetItemText(index,4,tmp);
				m_cSSTDTable.SetItemData(index,(DWORD_PTR)psi);
				index++;
			}
		}
		else
		{
			tmp.Format(L"%d",psi->Index);
			m_cSSTDTable.InsertItem(index,tmp);
			tmp.Format(L"%S",psi->FunctionName);
			m_cSSTDTable.SetItemText(index,1,tmp);
			tmp.Format(L"%p",psi->CurrentAddress);
			m_cSSTDTable.SetItemText(index,2,tmp);
			tmp.Format(L"%p",psi->OriginalAddress);
			m_cSSTDTable.SetItemText(index,3,tmp);
			tmp.Format(L"%s",psi->FullDllName);
			m_cSSTDTable.SetItemText(index,4,tmp);
			m_cSSTDTable.SetItemData(index,(DWORD_PTR)psi);
			index++;
		}
	}

}

void CSSDTView::OnNMCustomdrawSsdttable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pNMLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	*pResult = CDRF_DODEFAULT;

	if ( CDDS_PREPAINT == pNMLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pNMLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pNMLVCD->nmcd.dwDrawStage )
	{
		int item = pNMLVCD->nmcd.dwItemSpec;
		PSSDTInfo psi = (PSSDTInfo)m_cSSTDTable.GetItemData(item);
		if(psi->CurrentAddress!=psi->OriginalAddress)
		{
			pNMLVCD->clrText = RGB(255,0,0);
			pNMLVCD->clrTextBk = RGB(255,255,255);
		}
	}	
}

void CSSDTView::OnNMRClickSsdttable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POINT point; 
	GetCursorPos(&point);
	OnContextMenu(point);
	*pResult = 0;
}

void CSSDTView::OnContextMenu(CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_SSDT_MENU, point.x, point.y, this, TRUE);
}

void CSSDTView::OnUpdateUI(CCmdUI* pCui)
{
	pCui->Enable(TRUE);
}

void CSSDTView::OnRefresh()
{
	if(GetDocument()->IsSSDTShadow())
	{
		CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTShadowList",CRConsole::DoSSDTShadowList,GetDocument());
	}
	else
	{
		CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTList",CRConsole::DoSSDTList,GetDocument());
	}

	InitTable();
}

void CSSDTView::OnShowAll()
{
	GetDocument()->SetShowMode(TRUE);
	InitTable();
}

void CSSDTView::OnShowHook()
{
	GetDocument()->SetShowMode(FALSE);
	InitTable();
}

void CSSDTView::OnRecover()
{
	POSITION pos = m_cSSTDTable.GetFirstSelectedItemPosition();
	while(pos)
	{
		int iItem = m_cSSTDTable.GetNextSelectedItem(pos);
		PSSDTInfo psi = (PSSDTInfo)m_cSSTDTable.GetItemData(iItem);
		if(psi && psi->CurrentAddress!=psi->OriginalAddress)
		{
			if(GetDocument()->IsSSDTShadow())
			{
				CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTShadowRecover",CRConsole::DoSSDTShadowRecover,psi);
			}
			else
			{
				CTaskManager::GetInstance()->CreateTaskUI(L"DoSSDTRecover",CRConsole::DoSSDTRecover,psi);
			}
		}
	}
	OnRefresh();
}