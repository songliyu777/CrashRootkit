
// LoginDialogDlg.h : 头文件
//
#include "resource.h"
#include <afxdhtml.h>        // HTML 对话框
#pragma once


// CLoginDialog 对话框
class CLoginDialog : public CDHtmlDialog
{
// 构造
public:
	CLoginDialog(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_LOGINDIALOGDIALOG, IDH = IDR_HTML_LOGINDIALOG};

public:
	CString GetAccount();
	CString GetPassword();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// 实现
protected:
	HICON m_hIcon;

	CString m_account;
	CString m_password;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
