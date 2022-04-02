
// LoginDialogDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LoginDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLoginDialog 对话框

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
	DDX_DHtml_ElementValue(pDX,L"Account",m_account); //对应 input1
	DDX_DHtml_ElementValue(pDX,L"Password",m_password); //对应 input2
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDHtmlDialog)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CLoginDialog 消息处理程序

BOOL CLoginDialog::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLoginDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDHtmlDialog::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLoginDialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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