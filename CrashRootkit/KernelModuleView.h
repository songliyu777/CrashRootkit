#pragma once
#include "afxcmn.h"
#include "KernelModuleDoc.h"


// CKernelModuleView ������ͼ

class CKernelModuleView : public CFormView
{
	DECLARE_DYNCREATE(CKernelModuleView)

protected:
	CKernelModuleView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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


