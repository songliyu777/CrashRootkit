#include "StdAfx.h"
extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include "LuaEngine.h"
#include "Login.h"
#include "WebServiceClient.h"
#include "Util.h"
#include <string>
#include <iostream>
#include <sstream>
#include "IPMAC.h"

using namespace std;

#ifdef RELEASE_VERSION
#include "VMProtectSDK.h"
#endif

char * g_login_ok_str = "x3x8xex7x31x30x4x6b";
char * g_login_str = "x3x8xex7x6e";
char * g_business_str = "x17x6x1ax7xbx16x0x73";
char * g_statistics_str = "x7x15x15x1dx1ax7x1dxax10x73";
char * g_statistics_ok_str = "x7x15x15x1dx1ax7x1dxax10x2cx30x4x6b";
char * g_hlogin_str = "x4x3x8xex7x6e";
char * g_hlogin_ok_str = "x4x3x8xex7x31x30x4x6b";
char * g_binding_str = "xbx7xaxdx7x9x67";

CLogin * CLogin::m_instance = NULL;

CLogin g_login;

CLogin::CLogin(void)
{
	pdlg = NULL;
	m_instance = this;
}

CLogin::~CLogin(void)
{
	if(pdlg)
		delete pdlg;
}

CLogin * CLogin::GetInstance()
{
	return m_instance;
}

void CLogin::Init()
{
#ifdef RELEASE_VERSION
	VMProtectBegin("Init");
#endif
	RunScript("RegisterEx(27,\"AddProtectProcess\");");
	RunScript("RegisterEx(35,\"Exit\");");
	RunScript("RegisterEx(36,\"CreateProcess\");");
	RunScript("RegisterEx(44,\"Binding\");");
	char buf[512];
	sprintf(buf,"AddProtectProcess(%d);",GetCurrentProcessId());
	RunScript(buf);
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
}

bool CLogin::ShowLoginDialog()
{
	if(!pdlg)
		pdlg = new CLoginDialog();
	char account[45]={0};
	char password[45]={0};
	INT_PTR nResponse = pdlg->DoModal();
	if(nResponse==IDOK)
	{
#ifdef RELEASE_VERSION
		VMProtectBegin("ShowLoginDialog");
#endif
		WideCharToChar(pdlg->GetAccount().GetBuffer(),account,45);
		WideCharToChar(pdlg->GetPassword().GetBuffer(),password,45);
		string accountstr = account;
		string passwordstr = password;
		string loginstr = g_login_str;
		DecryptString(loginstr);
		string businessstr = g_business_str;
		DecryptString(businessstr);
		string result = Validation(accountstr,passwordstr,loginstr,businessstr,"4","");
		string loginokstr = g_login_ok_str;
		DecryptString(loginokstr);
#ifdef RELEASE_VERSION
		VMProtectEnd();
#endif
		if(!result.compare(loginokstr))
		{
			return true;
		}
	}
	return false;
}

bool CLogin::RunScript(const char * script)
{
	int error;
	error = luaL_loadbuffer(CLuaEngine::GetInstance()->GetLua_State(), script, strlen(script),
		"line") || lua_pcall(CLuaEngine::GetInstance()->GetLua_State(), 0, 0, 0);
	if (error) {
		return false;
	}
	return true;
}

void GetCPUID(OUT char * cpuid)
{
#ifdef RELEASE_VERSION
	VMProtectBegin("GetCPUID");
#endif
	unsigned long s1,s2,s3,s4;
	__asm
	{
		mov eax,00h
			xor edx,edx
			cpuid
			mov s1,edx
			mov s2,eax
	}
	__asm
	{
		mov eax,01h
			xor ecx,ecx
			xor edx,edx
			cpuid
			mov s3,edx
			mov s4,ecx
	}
	sprintf_s(cpuid,33,"%08X%08X%08X%08X",s1,s2,s3,s4);
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
}

bool CLogin::ValidateRegister()
{
#ifdef RELEASE_VERSION
#ifdef RELEASE_VERSION
	VMProtectBegin("ValidateRegister");
#endif
	char * account=ACCOUNT;
	char * password=PASSWORD;
	string cidstr = "";
	string ip = GetLocalIP();
	if(!ip.empty())
	{
		cidstr = GetMac(ip);
	}
	if(cidstr.empty())
	{
		char cpuid[33];
		GetCPUID(cpuid);
		cidstr = cpuid;
	}
	string accountstr = account;
	DecryptString(accountstr);
	string passwordstr = password;
	DecryptString(passwordstr);
	string businessstr = g_business_str;
	DecryptString(businessstr);
	string result = Validation(accountstr,passwordstr,businessstr,"1",cidstr,"");
	if(result.length()<20)
	{
		MessageBoxA(NULL,result.c_str(),"提示",0);
	}
	DecryptString(result);
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
#else
	string result = "RegisterEx(0,\"print\");RegisterEx(1,\"PRINT_ON\");RegisterEx(2,\"PRINT_OFF\");RegisterEx(3,\"ProcessesList\");RegisterEx(4,\"KernelHookCheck\"); \
					 RegisterEx(5,\"ModulesList\");RegisterEx(6,\"ThreadsList\");RegisterEx(7,\"HookCheckList\");RegisterEx(8,\"KernelModulesList\");RegisterEx(9,\"SSDTList\"); \
					 RegisterEx(10,\"SSDTShadowList\");RegisterEx(11,\"DpcTimerList\");RegisterEx(12,\"IoTimerList\");RegisterEx(13,\"AntiProtectHook1\");RegisterEx(14,\"AntiProtectHook2\"); \
					 RegisterEx(15,\"GetProcAddressByName\");RegisterEx(16,\"StopIoTimer\");RegisterEx(17,\"KillDpcTimer\");RegisterEx(18,\"KernelHookRecover\");RegisterEx(19,\"TerminateThread\"); \
					 RegisterEx(20,\"WriteProcessMemory\");RegisterEx(21,\"KdDisableDebugger\");RegisterEx(22,\"KdEnableDebugger\");RegisterEx(23,\"HideKernelDebugger\");RegisterEx(24,\"MonitorDriverLoad\");\
					 RegisterEx(25,\"AddTargetProcess\");RegisterEx(26,\"RecoverObject\");RegisterEx(27,\"AddProtectProcess\");RegisterEx(28,\"AddTargetKernelModule\");RegisterEx(29,\"AddFilterDrvIo\");\
					 RegisterEx(30,\"TerminateProcess\");RegisterEx(31,\"HideProcessModule\");RegisterEx(32,\"HideProcessMemory\");RegisterEx(33,\"SetKeSpeed\");RegisterEx(34,\"SetKeException\");\
					 RegisterEx(37,\"Statistics\");RegisterEx(38,\"AddSeparateProcess\");RegisterEx(39,\"Simulate\");RegisterEx(40,\"SuspendProcess\");RegisterEx(41,\"ResumeProcess\");\
					 RegisterEx(42,\"Shutdown\");RegisterEx(43,\"HLogin\");RegisterEx(44,\"Binding\");RegisterEx(45,\"HideKernelModule\");RegisterEx(46,\"ModifyKernelMemory\")";
#endif
	return RunScript(result.c_str());
}

template <class T> 
string ConvertToString(T value) {
	stringstream ss;
	ss << value;
	return ss.str();
}


bool CLogin::Statistics(const char * GameAccount,const char * RoleName,double Money)
{
#ifdef RELEASE_VERSION
	VMProtectBegin("Statistics");
#endif
	char * account=ACCOUNT;
	char * password=PASSWORD;
	string accountstr = account;
	DecryptString(accountstr);
	string passwordstr = password;
	DecryptString(passwordstr);
	string statisticsstr = g_statistics_str;
	string rolenamestr = RoleName;
	string gameaccountstr = GameAccount; 
	string moneystr = ConvertToString(Money);
	DecryptString(statisticsstr);
	string result = Validation(accountstr,passwordstr,statisticsstr,gameaccountstr,rolenamestr,moneystr);
	string statisticsokstr = g_statistics_ok_str;
	DecryptString(statisticsokstr);
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
	if(!result.compare(statisticsokstr))
	{
		return true;
	}
	return true;
}

bool CLogin::Hack()
{
#ifdef RELEASE_VERSION
	VMProtectBegin("HLogin");
#endif
	char * account=ACCOUNT;
	char * password=PASSWORD;
	string cidstr = "";
	string ip = GetLocalIP();
	if(!ip.empty())
	{
		cidstr = GetMac(ip);
	}
	if(cidstr.empty())
	{
		char cpuid[33];
		GetCPUID(cpuid);
		cidstr = cpuid;
	}
	string accountstr = account;
	DecryptString(accountstr);
	string passwordstr = password;
	DecryptString(passwordstr);
	string hloginstr = g_hlogin_str;
	DecryptString(hloginstr);
	string result = Validation(accountstr,passwordstr,hloginstr,"1",cidstr,"");
	string hloginokstr = g_hlogin_ok_str;
	DecryptString(hloginokstr);
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
	if(!result.compare(hloginokstr))
	{
		return true;
	}
	return false;
}

bool CLogin::Binding(bool binding)
{
#ifdef RELEASE_VERSION
	VMProtectBegin("Binding");
#endif
	char * account=ACCOUNT;
	char * password=PASSWORD;
	string cidstr = "";
	string ip = GetLocalIP();
	if(!ip.empty())
	{
		cidstr = GetMac(ip);
	}
	if(cidstr.empty())
	{
		char cpuid[33];
		GetCPUID(cpuid);
		cidstr = cpuid;
	}
	string accountstr = account;
	DecryptString(accountstr);
	string passwordstr = password;
	DecryptString(passwordstr);
	string binding_str = g_binding_str;
	DecryptString(binding_str);
	string result;
	if(binding)
	{
		result = Validation(accountstr,passwordstr,binding_str,"1",cidstr,"true");
	}
	else
	{
		result = Validation(accountstr,passwordstr,binding_str,"1",cidstr,"false");
	}
	MessageBoxA(NULL,result.c_str(),"提示",0);
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
	return true;
}