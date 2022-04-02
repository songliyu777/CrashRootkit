#pragma once
#include "afxcmn.h"
#include "KernelModuleDoc.h"


// CKernelModuleView 窗体视图

class CKernelModuleView : public CFormView
{
	DECLARE_DYNCREATE(CKernelModuleView)

protected:
	CKernelModuleView();           // 动态创建所使用的受保护的构造函数
	virtual ~CKernelModuleView();

public:
	enum { IDD = IDD_KERNELMODULEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	CKernelModuleDoc* GetDocument() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_cKernelmoduleTable;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	void InitTable();
	afx_msg void OnNMRClickKernelmoduletable(NMHDR *pNMHDR, LRESULT *pResult);
	void OnContextMenu(CPoint point);;
	afx_msg void OnUpdateUI(CCmdUI* pCui);
	void OnRefresh();
};


