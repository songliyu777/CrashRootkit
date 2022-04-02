#pragma once
#include "afxcmn.h"
#include "Transfer.h"
#include "ProcessDoc.h"


// CModulesPage 对话框

class CModulesPage : public CDialog
{
	DECLARE_DYNAMIC(CModulesPage)

public:
	CModulesPage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModulesPage();

// 对话框数据
	enum { IDD = IDD_MODULESPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT DoUpdateData(WPARAM wParam, LPARAM lParam);
	CListCtrl m_cModulesTable;
	void InitTable(ModuleInfoVector *pmiv);
	afx_msg void OnNMRClickModulestable(NMHDR *pNMHDR, LRESULT *pResult);
};
