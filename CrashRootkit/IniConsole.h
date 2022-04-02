#pragma once

typedef struct _GlobleInfo
{
	BOOL BIGMAP_UPDATE;
	BOOL MAP_UPDATE;
	BOOL ITEM_UPDATE;
	BOOL SKILL_UPDATE;
	_GlobleInfo()
	{
		BIGMAP_UPDATE = TRUE;
		MAP_UPDATE = TRUE;
		ITEM_UPDATE = TRUE;
		SKILL_UPDATE = TRUE;
	}
}GlobleInfo,*PGlobleInfo;

class CIniConsole
{
public:
	CIniConsole(void);
	LPCTSTR GetIniFilePath(LPCTSTR lpFileName);
	GlobleInfo GetGlobleIni();
	~CIniConsole(void);
};
