#include "StdAfx.h"
#include "ScriptFile.h"
#include "ConsoleDll.h"
#include "PETools_R3.h"
#include "Login.h"

bool RunLocateScript()
{
	char szDriverImagePath[MAX_PATH] = {0};
	GetModuleFileNameA(NULL,szDriverImagePath,MAX_PATH); //得到当前模块路径
	for (int i=strlen(szDriverImagePath);i>0;i--)
	{
		if ('\\'==szDriverImagePath[i])
		{
			szDriverImagePath[i+1]='\0';
			break;
		}
	}
	char path[MAX_PATH]={0};
	char tmp[MAX_PATH]={0};
	int len = strlen(szDriverImagePath);
	int index = 0;
	for (int i= 0;i<len;i++)
	{
		if ('\\'==szDriverImagePath[i])
		{
			ZeroMemory(tmp,MAX_PATH);
			strncpy_s(tmp,szDriverImagePath + index,i-index);
			strcat_s(path,tmp);
			strcat_s(path,"\\\\");
			index = i + 1;
		}
	}
	strcat_s(path,"AliWorkbench.exe");
	char buf[512];
	sprintf_s(buf,"CreateProcess(\"%s\",\"\");",path);
	return CLogin::GetInstance()->RunScript(buf);
}
