
// CrashRootkit.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "CrashRootkit.h"
#include "MainFrm.h"
#include "Platform.h"
#include "LuaEngine.h"
#include "ChildFrm.h"
#include "SplitterFrame.h"
#include "CrashRootkitDoc.h"
#include "CrashRootkitView.h"
#include "SSDTDoc.h"
#include "SSDTView.h"
#include "ConsoleDll.h"
#include "CRDocManager.h"
#include "CRMultiDocTemplate.h"
#include "KernelHookDoc.h"
#include "KernelHookView.h"
#include "ProcessView.h"
#include "KernelModuleView.h"
#include "KernelModuleDoc.h"
#include "DpcTimerDoc.h"
#include "DpcTimerView.h"
#include "IoTimerDoc.h"
#include "IoTimerView.h"
#include "PETools_R3.h"
#include "ProcessTools.h"
#include "ScriptFile.h"
#include "TCServiceConsole.h"
#include "CRConsole.h"
#include "FlashDialog.h"
#include "CRUpdate.h"
#include "IPMAC.h"
#include "IniConsole.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Login.h"
// CCrashRootkitApp

BEGIN_MESSAGE_MAP(CCrashRootkitApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CCrashRootkitApp::OnAppAbout)
	// �����ļ��ı�׼�ĵ�����
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// ��׼��ӡ��������
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CCrashRootkitApp ����

CCrashRootkitApp::CCrashRootkitApp()
{

	m_bHiColorIcons = TRUE;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}

// Ψһ��һ�� CCrashRootkitApp ����

CCrashRootkitApp theApp;

// CCrashRootkitApp ��ʼ��

BOOL CCrashRootkitApp::InitInstance()
{
	if(IsWow64())
	{
		AfxMessageBox(L"�ݲ�֧�ֵ�64λ����ϵͳ");
		return FALSE;
	}
	SYSTEM_VERSION version = GetSystemVersion();
	switch(version)
	{
	case WINDOWSXP:
	case WINDOWS7:
		break;
	case WINDOWS8:
	case WINDOWS2003:
	case WINDOWSVISTA:
		AfxMessageBox(L"��֧�ֵĲ���ϵͳ");
		return FALSE;
	}
	//CLuaEngine::GetInstance()->RunScript();
	//if(CheckCRExist())
	//{
	//	AfxMessageBox(L"ֻ�����һ������");
	//	return FALSE;
	//}
#ifdef _SIMPLE
#ifdef SAMUELSONG
	char * url = "http://7501.s30.javaidc.com/update_test/";
#else
	char * url = "http://7501.s30.javaidc.com/update_zs_10/";
#endif
	CWebConfig * pWebConfig = LoadWebConfig(url);
	if(!pWebConfig)
		return FALSE;
	SetHistance(HINSTANCE(GetModuleHandle(L"drivergenius.exe")));
	RecoverLogo(pWebConfig,url);
	ShowFlashDialog();
	Sleep(1000);
	RecoverFile(pWebConfig,url);
	CloseFlashDialog();
	delete pWebConfig;
#endif
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	//�����ܵ�
	if(!TCServiceConsole::GetInstance()->StartService())
	{
		return FALSE;
	}
	InitCommonControlsEx(&InitCtrls);
	//��������ʧ��
	if(!InitializeCRD())
	{
		return FALSE;
	}
	if(!SetLogPath())
	{
		ReleaseCRD();
		return FALSE;
	}
	//�����������
	LOGFONT logfont = {0};
	:: SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &logfont, 0);
	afxGlobalData.SetMenuFont(&logfont,true);

	if(afxGlobalData.fontTooltip.GetSafeHandle() != NULL)
	{
		::DeleteObject(afxGlobalData.fontTooltip.Detach());
	}
	afxGlobalData.fontTooltip.CreateFontIndirect(&logfont);

	CWinAppEx::InitInstance();

	// ��ʼ�� OLE ��
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
#ifdef RELEASE_VERSION
	//if(!CLogin::GetInstance()->ShowLoginDialog())
	//{
	//	ReleaseCRD();
	//	return FALSE;
	//}
#endif

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));
	LoadStdProfileSettings(4);  // ���ر�׼ INI �ļ�ѡ��(���� MRU)

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// ע��Ӧ�ó�����ĵ�ģ�塣�ĵ�ģ��
	// �������ĵ�����ܴ��ں���ͼ֮�������
	CCRMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CCRMultiDocTemplate(IDR_CrashRootkitTYPE,
		RUNTIME_CLASS(CCrashRootkitDoc),
		RUNTIME_CLASS(CSplitterFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CEditView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_KERNELMODULETYPE,
		RUNTIME_CLASS(CKernelModuleDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CKernelModuleView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_KERNELHOOKTYPE,
		RUNTIME_CLASS(CKernelHookDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CKernelHookView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_PROCESSTYPE,
		RUNTIME_CLASS(CProcessDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CProcessView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_SSDTTYPE,
		RUNTIME_CLASS(CSSDTDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CSSDTView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_SSDTSHADOWTYPE,
		RUNTIME_CLASS(CSSDTDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CSSDTView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_DPCTIMERTYPE,
		RUNTIME_CLASS(CDpcTimerDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CDpcTimerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_IOTIMERTYPE,
		RUNTIME_CLASS(CIoTimerDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CIoTimerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// ������ MDI ��ܴ���
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
#ifdef _SIMPLE
	pMainFrame->ShowWindow(SW_HIDE);
#endif
	// �������к�׺ʱ�ŵ��� DragAcceptFiles
	//  �� MDI Ӧ�ó����У���Ӧ������ m_pMainWnd ֮����������


	// ������׼������DDE�����ļ�������������
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	
	CLogin::GetInstance()->Init();
	CLogin::GetInstance()->ValidateRegister();

	// ��������������ָ����������
	// �� /RegServer��/Register��/Unregserver �� /Unregister ����Ӧ�ó����򷵻� FALSE��
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// �������ѳ�ʼ���������ʾ����������и���
#ifndef _SIMPLE
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
#else
	return RunLocateScript();
#endif
	return TRUE;
}

int CCrashRootkitApp::ExitInstance()
{
	ReleaseCRD();
	return CWinAppEx::ExitInstance();
}


void CCrashRootkitApp::AddDocTemplate(CDocTemplate* pTemplate)
{
	if (m_pDocManager == NULL)
		m_pDocManager = new CCRDocManager;
	m_pDocManager->AddDocTemplate(pTemplate);
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CCrashRootkitApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCrashRootkitApp �Զ������/���淽��

void CCrashRootkitApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CCrashRootkitApp::LoadCustomState()
{
}

void CCrashRootkitApp::SaveCustomState()
{
}

// CCrashRootkitApp ��Ϣ�������



