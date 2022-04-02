#include "StdAfx.h"
#include "LuaEngine.h"
extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "Util.h"
#include "Log.h"
#include "LuaFunction.h"
#include "CRConsole.h"
#include "CRString.h"
#include <string>
using namespace std;

CLuaEngine * CLuaEngine::m_instance = NULL;

CLuaEngine g_LuaEngine;

lua_State *L = NULL;

CLuaEngine::CLuaEngine(void)
{
	assert(!m_instance);
	m_scriptbuff = (LPSTR)malloc(MAXSCRIPTBUFF);
	assert(m_scriptbuff);
	m_instance = this;
	L = luaL_newstate();
	luaL_openlibs(L);
	RegisterFunction(L);
}

CLuaEngine::~CLuaEngine(void)
{
	lua_close(L);
	free((void *)m_scriptbuff);
}

CLuaEngine * CLuaEngine::GetInstance()
{
	return m_instance;
}

lua_State * CLuaEngine::GetLua_State()
{
	return L;
}

bool CLuaEngine::RunScript()
{
	//string tmpstr = m_scriptbuff;
	//tmpstr = "abcd1234";
	//EncryptString(tmpstr);
#ifdef RELEASE_VERSION
	if(IsFindRegisterFunction(m_scriptbuff))
	{
		return false;
	}
#endif
	wchar_t tmp[512];
	int error;
	error = luaL_loadbuffer(L, m_scriptbuff, strlen(m_scriptbuff),
			"line") || lua_pcall(L, 0, 0, 0);
	if (error) {
		const char * cerror = lua_tostring(L, -1);
		CString errorstr;
		CharToWideChar(cerror,tmp,512);
		errorstr.Format(L"%s\r\n",tmp);
		m_outputview->AddOutputString(errorstr);
		lua_pop(L, 1);/* pop error message from the stack */
		return false;
	}
	return true;
}

LPSTR CLuaEngine::GetScriptBuff()
{
	return m_scriptbuff;
}

void CLuaEngine::ClearScriptBuff()
{
	ZeroMemory((void *)m_scriptbuff,MAXSCRIPTBUFF);
}

