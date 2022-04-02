
// CrashRootkitView.h : CCrashRootkitView 类的接口
//


#pragma once
#include "CrashRootkitDoc.h"

class CCrashRootkitView : public CFormView
{
protected: // 仅从序列化创建
	CCrashRootkitView();
	DECLARE_DYNCREATE(CCrashRootkitView)

// 属性
public:
	CCrashRootkitDoc* GetDocument() const;

// 操作
public:
	enum { IDD = IDD_CRVIEW };
// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
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
// 实现
public:
	virtual ~CCrashRootkitView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
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

#ifndef _DEBUG  // CrashRootkitView.cpp 中的调试版本
inline CCrashRootkitDoc* CCrashRootkitView::GetDocument() const
   { return reinterpret_cast<CCrashRootkitDoc*>(m_pDocument); }
#endif

