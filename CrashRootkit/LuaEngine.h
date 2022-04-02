#pragma once

#include "ScriptEditView.h"
#include "OutputView.h"
extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class CLuaEngine
{
	static CLuaEngine * m_instance;
	LPSTR m_scriptbuff;
public:
	CLuaEngine(void);
	~CLuaEngine(void);
	static CLuaEngine * CLuaEngine::GetInstance();
	bool RunScript();
	LPSTR GetScriptBuff();
	void ClearScriptBuff();
	CScriptEditView * m_scriptview;
	COutputView * m_outputview;
	lua_State * GetLua_State();
};

#define MAXSCRIPTBUFF 1024*1024