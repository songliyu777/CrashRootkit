#pragma once
#include "afxcmn.h"
#include "Transfer.h"

// CHookCheckPage 对话框

class CHookCheckPage : public CDialog
{
	DECLARE_DYNAMIC(CHookCheckPage)

public:
	CHookCheckPage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHookCheckPage();

// 对话框数据
	enum { IDD = IDD_HOOKCHECKPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT DoUpdateData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CPoint point);
	afx_msg void OnRefresh();
	afx_msg void OnRecover();

	CListCtrl m_hookchecktable;

	void InitTable(MemoryHookVector *pmhv);
	afx_msg void OnNMRClickHookchecktable(NMHDR *pNMHDR, LRESULT *pResult);
};
