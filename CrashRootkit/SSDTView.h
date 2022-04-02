#pragma once
#include "resource.h"
#include "SSDTDoc.h"
#include "afxcmn.h"

class CSSDTView : public CFormView
{
public:
	CSSDTView(void);
	virtual ~CSSDTView(void);

public:
	enum { IDD = IDD_SSDTVIEW };

	CSSDTDoc* GetDocument() const;

	virtual void OnInitialUpdate();

	void InitTable();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Ö§³Ö
	DECLARE_DYNCREATE(CSSDTView)
	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CPoint point);
	afx_msg void OnRefresh();
	afx_msg void OnShowAll();
	afx_msg void OnShowHook();
	afx_msg void OnRecover();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CListCtrl m_cSSTDTable;
	afx_msg void OnNMCustomdrawSsdttable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickSsdttable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateUI(CCmdUI* pCui);
};
