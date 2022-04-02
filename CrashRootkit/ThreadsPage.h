#pragma once
#include "afxcmn.h"
#include "Transfer.h"


// CThreadsPage 对话框

class CThreadsPage : public CDialog
{
	DECLARE_DYNAMIC(CThreadsPage)

public:
	CThreadsPage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CThreadsPage();

// 对话框数据
	enum { IDD = IDD_THREADSPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CListCtrl m_cThreadsTable;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT DoUpdateData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRefresh();
	afx_msg void OnTerminate();
	afx_msg void OnContextMenu(CPoint point);
	void InitTable(ThreadInfoVector *ptiv);
	afx_msg void OnNMRClickThreadstable(NMHDR *pNMHDR, LRESULT *pResult);
};
