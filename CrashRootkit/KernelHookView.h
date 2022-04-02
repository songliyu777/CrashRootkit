#pragma once
#include "resource.h"
#include "afxext.h"
#include "KernelHookDoc.h"
#include "afxcmn.h"

class CKernelHookView :
	public CFormView
{
public:
	CKernelHookView(void);
	~CKernelHookView(void);

	enum { IDD = IDD_KERNELHOOKVIEW };

	CKernelHookDoc* GetDocument() const;

	virtual void OnInitialUpdate();

	void InitTable();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV Ö§³Ö
	DECLARE_DYNCREATE(CKernelHookView)
	DECLARE_MESSAGE_MAP()

	afx_msg void OnContextMenu(CPoint point);
	afx_msg void OnRefresh();
	afx_msg void OnRecover();
public:
	CListCtrl n_cKernelHookTable;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateUI(CCmdUI* pCui);
	afx_msg void OnNMRClickKernelhooktable(NMHDR *pNMHDR, LRESULT *pResult);
};
