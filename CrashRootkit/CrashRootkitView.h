
// CrashRootkitView.h : CCrashRootkitView ��Ľӿ�
//


#pragma once
#include "CrashRootkitDoc.h"

class CCrashRootkitView : public CFormView
{
protected: // �������л�����
	CCrashRootkitView();
	DECLARE_DYNCREATE(CCrashRootkitView)

// ����
public:
	CCrashRootkitDoc* GetDocument() const;

// ����
public:
	enum { IDD = IDD_CRVIEW };
// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

protected:
	COLORREF m_bg_LineNumber;
	COLORREF m_fg_LineNumber;
	DWORD m_width_LineNumber;
	void DrawLineNumber(CDC* pDC);
// ʵ��
public:
	virtual ~CCrashRootkitView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // CrashRootkitView.cpp �еĵ��԰汾
inline CCrashRootkitDoc* CCrashRootkitView::GetDocument() const
   { return reinterpret_cast<CCrashRootkitDoc*>(m_pDocument); }
#endif

