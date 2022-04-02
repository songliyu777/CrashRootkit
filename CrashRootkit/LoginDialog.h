
// LoginDialogDlg.h : ͷ�ļ�
//
#include "resource.h"
#include <afxdhtml.h>        // HTML �Ի���
#pragma once


// CLoginDialog �Ի���
class CLoginDialog : public CDHtmlDialog
{
// ����
public:
	CLoginDialog(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_LOGINDIALOGDIALOG, IDH = IDR_HTML_LOGINDIALOG};

public:
	CString GetAccount();
	CString GetPassword();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// ʵ��
protected:
	HICON m_hIcon;

	CString m_account;
	CString m_password;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
