#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "ProgressCtrlEx.h"

// CProgressDlg �Ի���
class CTask;

class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CProgressDlg();

// �Ի�������
	enum { IDD = IDD_PROGRESS_DLG };

	DWORD m_value;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	BOOL m_bUnknow;
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	VOID SetUnknow(BOOL bUnknow);
	CProgressCtrlEx m_progress_bar;
	CTask * m_pTask;
};
