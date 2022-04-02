
// CrashRootkitView.cpp : CCrashRootkitView ���ʵ��
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
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CCrashRootkitView::OnFilePrintPreview)
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CCrashRootkitView ����/����

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
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CCrashRootkitView ����

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


// CCrashRootkitView ��ӡ


void CCrashRootkitView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

BOOL CCrashRootkitView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CCrashRootkitView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CCrashRootkitView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
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


// CCrashRootkitView ���

#ifdef _DEBUG
void CCrashRootkitView::AssertValid() const
{
	CView::AssertValid();
}

void CCrashRootkitView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCrashRootkitDoc* CCrashRootkitView::GetDocument() const // �ǵ��԰汾��������
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

// CCrashRootkitView ��Ϣ�������

void CCrashRootkitView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	// TODO: �ڴ����ר�ô����/����û���
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
