#pragma once
#include "afxcmn.h"
#include "Transfer.h"
#include "ProcessDoc.h"


// CModulesPage �Ի���

class CModulesPage : public CDialog
{
	DECLARE_DYNAMIC(CModulesPage)

public:
	CModulesPage(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CModulesPage();

// �Ի�������
	enum { IDD = IDD_MODULESPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT DoUpdateData(WPARAM wParam, LPARAM lParam);
	CListCtrl m_cModulesTable;
	void InitTable(ModuleInfoVector *pmiv);
	afx_msg void OnNMRClickModulestable(NMHDR *pNMHDR, LRESULT *pResult);
};
