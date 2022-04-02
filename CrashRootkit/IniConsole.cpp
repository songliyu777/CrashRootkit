#include "StdAfx.h"
#include "IniConsole.h"
#include "CRUpdate.h"

CIniConsole::CIniConsole(void)
{
}

CIniConsole::~CIniConsole(void)
{
}

LPCTSTR CIniConsole::GetIniFilePath(LPCTSTR lpFileName)
{
	TCHAR Buffer[MAX_PATH];
	static TCHAR SelectFile[MAX_PATH];
	DWORD dwRet = ::GetCurrentDirectory(MAX_PATH, Buffer);
	if (dwRet == 0)
	{
		TRACE("CAutoLogin::GetIniPathFile GetCurrentDirectory failed (%d)\r\n", GetLastError());
		return NULL;
	}
	if (dwRet > BUFSIZ)
	{
		TRACE("CAutoLogin::GetIniPathFile GetCurrentDirectory failed (buffer too small; need %d chars)\r\n", dwRet);
		return NULL;
	}
	swprintf_s(SelectFile, MAX_PATH, _T("%s\\ini\\%s"), Buffer, lpFileName);
	//判定INI文件是否存在
	if (!PathFileExists(SelectFile))
	{
		TRACE("CAutoLogin::GetIniPathFile PathFileExists: INI file is not exist!\r\n");
		return NULL;
	}

	return (LPCTSTR)SelectFile;
}

GlobleInfo CIniConsole::GetGlobleIni()
{
	GlobleInfo globleinfo;
	LPCTSTR lpstrLoginFile = GetIniFilePath(_T("globle.ini"));
	TRACE("LoginPath[%S]\r\n",lpstrLoginFile);
	if (lpstrLoginFile == NULL)
	{
		return globleinfo;
	}

	CString UpdateStr = _T("UPDATE");

	globleinfo.BIGMAP_UPDATE = GetPrivateProfileInt(UpdateStr, _T(BIGMAP),1,lpstrLoginFile);
	globleinfo.MAP_UPDATE = GetPrivateProfileInt(UpdateStr, _T(MAP),1,lpstrLoginFile);
	globleinfo.ITEM_UPDATE = GetPrivateProfileInt(UpdateStr, _T(ITEM),1,lpstrLoginFile);
	globleinfo.SKILL_UPDATE = GetPrivateProfileInt(UpdateStr, _T(SKILL),1,lpstrLoginFile);
	return globleinfo;
}
