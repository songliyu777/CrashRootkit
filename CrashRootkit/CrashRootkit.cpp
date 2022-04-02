
// CrashRootkit.cpp : 定义应用程序的类行为。
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
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CCrashRootkitApp 构造

CCrashRootkitApp::CCrashRootkitApp()
{

	m_bHiColorIcons = TRUE;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CCrashRootkitApp 对象

CCrashRootkitApp theApp;

// CCrashRootkitApp 初始化

BOOL CCrashRootkitApp::InitInstance()
{
	if(IsWow64())
	{
		AfxMessageBox(L"暂不支持的64位操作系统");
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
		AfxMessageBox(L"不支持的操作系统");
		return FALSE;
	}
	//CLuaEngine::GetInstance()->RunScript();
	//if(CheckCRExist())
	//{
	//	AfxMessageBox(L"只允许打开一个工具");
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
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	//命名管道
	if(!TCServiceConsole::GetInstance()->StartService())
	{
		return FALSE;
	}
	InitCommonControlsEx(&InitCtrls);
	//驱动加载失败
	if(!InitializeCRD())
	{
		return FALSE;
	}
	if(!SetLogPath())
	{
		ReleaseCRD();
		return FALSE;
	}
	//解决字体问题
	LOGFONT logfont = {0};
	:: SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &logfont, 0);
	afxGlobalData.SetMenuFont(&logfont,true);

	if(afxGlobalData.fontTooltip.GetSafeHandle() != NULL)
	{
		::DeleteObject(afxGlobalData.fontTooltip.Detach());
	}
	afxGlobalData.fontTooltip.CreateFontIndirect(&logfont);

	CWinAppEx::InitInstance();

	// 初始化 OLE 库
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

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CCRMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CCRMultiDocTemplate(IDR_CrashRootkitTYPE,
		RUNTIME_CLASS(CCrashRootkitDoc),
		RUNTIME_CLASS(CSplitterFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CEditView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_KERNELMODULETYPE,
		RUNTIME_CLASS(CKernelModuleDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CKernelModuleView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_KERNELHOOKTYPE,
		RUNTIME_CLASS(CKernelHookDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CKernelHookView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_PROCESSTYPE,
		RUNTIME_CLASS(CProcessDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CProcessView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_SSDTTYPE,
		RUNTIME_CLASS(CSSDTDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CSSDTView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_SSDTSHADOWTYPE,
		RUNTIME_CLASS(CSSDTDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CSSDTView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_DPCTIMERTYPE,
		RUNTIME_CLASS(CDpcTimerDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CDpcTimerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CCRMultiDocTemplate(IDR_IOTIMERTYPE,
		RUNTIME_CLASS(CIoTimerDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CIoTimerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// 创建主 MDI 框架窗口
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
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 MDI 应用程序中，这应在设置 m_pMainWnd 之后立即发生


	// 分析标准外壳命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	
	CLogin::GetInstance()->Init();
	CLogin::GetInstance()->ValidateRegister();

	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 主窗口已初始化，因此显示它并对其进行更新
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

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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

// 用于运行对话框的应用程序命令
void CCrashRootkitApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCrashRootkitApp 自定义加载/保存方法

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

// CCrashRootkitApp 消息处理程序



