
// CrashRootkitView.cpp : CCrashRootkitView 类的实现
//

#include "stdafx.h"
#include "CrashRootkit.h"

#include "CrashRootkitDoc.h"
#include "CrashRootkitView.h"
#include "IoTimerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCrashRootkitView

IMPLEMENT_DYNCREATE(CCrashRootkitView, CFormView)

BEGIN_MESSAGE_MAP(CCrashRootkitView, CFormView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CCrashRootkitView::OnFilePrintPreview)
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CCrashRootkitView 构造/析构

CCrashRootkitView::CCrashRootkitView()
	: CFormView(CCrashRootkitView::IDD)
{
	m_bg_LineNumber = RGB(255,255,255);
	m_fg_LineNumber = RGB(0,0,0);
	m_width_LineNumber = 25;
}

CCrashRootkitView::~CCrashRootkitView()
{
}

BOOL CCrashRootkitView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CCrashRootkitView 绘制

void CCrashRootkitView::OnDraw(CDC* pDC)
{
	CCrashRootkitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rectClient;
	GetClientRect(&rectClient);

	CDC dc;	
	dc.CreateCompatibleDC(pDC);

	CBitmap bm;
	bm.CreateCompatibleBitmap(pDC, rectClient.Width(), rectClient.Height());
	dc.SelectObject(bm);
	dc.SetBoundsRect(&rectClient, DCB_DISABLE);

	DrawLineNumber(&dc);

	pDC->BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	dc.DeleteDC();
}


// CCrashRootkitView 打印


void CCrashRootkitView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CCrashRootkitView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CCrashRootkitView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CCrashRootkitView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CCrashRootkitView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CCrashRootkitView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CCrashRootkitView 诊断

#ifdef _DEBUG
void CCrashRootkitView::AssertValid() const
{
	CView::AssertValid();
}

void CCrashRootkitView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCrashRootkitDoc* CCrashRootkitView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCrashRootkitDoc)));
	return (CCrashRootkitDoc*)m_pDocument;
}
#endif //_DEBUG

void CCrashRootkitView::DrawLineNumber(CDC* pDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);
	rectClient.right = rectClient.left + m_width_LineNumber;
	CBrush bkBrush(m_bg_LineNumber);
	pDC->FillRect(&rectClient,&bkBrush);
}

// CCrashRootkitView 消息处理程序

void CCrashRootkitView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO: 在此添加专用代码和/或调用基类
}

int CCrashRootkitView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CCrashRootkitView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

}
