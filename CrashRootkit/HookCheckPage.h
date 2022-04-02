#pragma once
#include "afxcmn.h"
#include "Transfer.h"

// CHookCheckPage �Ի���

class CHookCheckPage : public CDialog
{
	DECLARE_DYNAMIC(CHookCheckPage)

public:
	CHookCheckPage(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CHookCheckPage();

// �Ի�������
	enum { IDD = IDD_HOOKCHECKPAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
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
