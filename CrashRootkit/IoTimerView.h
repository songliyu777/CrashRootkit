#pragma once
#include "afxcmn.h"
#include "IoTimerDoc.h"


// CIoTimerView 窗体视图

class CIoTimerView : public CFormView
{
	DECLARE_DYNCREATE(CIoTimerView)

protected:
	CIoTimerView();           // 动态创建所使用的受保护的构造函数
	virtual ~CIoTimerView();

public:
	enum { IDD = IDD_IOTIMERVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CIoTimerDoc* GetDocument() const;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CPoint point);
	afx_msg void OnRefresh();
	afx_msg void OnStart();
	afx_msg void OnStop();
public:
	virtual void OnInitialUpdate();
	CListCtrl m_IoTimerTable;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void InitTable();
	afx_msg void OnNMRClickIotimertable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateUI(CCmdUI* pCui);

};


