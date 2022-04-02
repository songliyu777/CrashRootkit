#pragma once
#include "resource.h"
#include "afxext.h"
#include "ProcessDoc.h"
#include "afxcmn.h"

#define WM_DOUPDATEDATA WM_USER + 1

class CProcessView :
	public CFormView
{
public:
	CProcessView(void);
	~CProcessView(void);
	
	enum { IDD = IDD_PROCESSVIEW };

	CProcessDoc* GetDocument() const;

	virtual void OnInitialUpdate();

	void InitTable();
	void CreatePage(int page);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Ö§³Ö
	DECLARE_DYNCREATE(CProcessView)
	DECLARE_MESSAGE_MAP()

public:
	CListCtrl m_cProcessTable;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CList<CDialog*,CDialog*> m_PageList;
	CTabCtrl m_cTabCtrl;
	afx_msg void OnNMClickProcesstable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickProcesstable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangeProcesstab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateUI(CCmdUI* pCui);
	afx_msg void OnRefresh();
	afx_msg void OnTerminate();
	void OnContextMenu(CPoint point);
};
