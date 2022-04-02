// ProgressDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CrashRootkit.h"
#include "ProgressDlg.h"
#include "Task.h"


// CProgressDlg �Ի���

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
{
	m_value = 0;
	m_pTask = NULL;
	m_bUnknow = TRUE;
}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progress_bar);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CProgressDlg::OnOK()
{
	CDialog::OnOK();
}

void CProgressDlg::OnCancel()
{
	m_pTask->Stop();
	CDialog::OnCancel();
}

// CProgressDlg ��Ϣ�������

void CProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_progress_bar.SetPos(m_value);
	if(m_value == 100)
	{
		CDialog::OnCancel();
	}
	CDialog::OnTimer(nIDEvent);
}

BOOL CProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_progress_bar.GetSafeHwnd())
	{
		m_progress_bar.SetRange(0,100);
		if(m_bUnknow)
		{
			m_progress_bar.SetStyle(m_progress_bar.GetStyle() | PBS_GLIDING);
		}
	}

	SetWindowText(m_pTask->GetTaskName());
	SetTimer(1,100,NULL);

	return TRUE;  
}

VOID CProgressDlg::SetUnknow(BOOL bUnknow)
{
	m_bUnknow = bUnknow;
}