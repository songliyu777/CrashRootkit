
// LoginDialogDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "LoginDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLoginDialog �Ի���

BEGIN_DHTML_EVENT_MAP(CLoginDialog)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



CLoginDialog::CLoginDialog(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CLoginDialog::IDD, CLoginDialog::IDH, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLoginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_DHtml_ElementValue(pDX,L"Account",m_account); //��Ӧ input1
	DDX_DHtml_ElementValue(pDX,L"Password",m_password); //��Ӧ input2
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDHtmlDialog)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CLoginDialog ��Ϣ�������

BOOL CLoginDialog::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CLoginDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDHtmlDialog::OnSysCommand(nID, lParam);
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CLoginDialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CLoginDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HRESULT CLoginDialog::OnButtonOK(IHTMLElement* /*pElement*/)
{
	UpdateData();
	OnOK();
	return S_OK;
}

HRESULT CLoginDialog::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}

CString CLoginDialog::GetAccount()
{
	return m_account;
}

CString CLoginDialog::GetPassword()
{
	return m_password;
}