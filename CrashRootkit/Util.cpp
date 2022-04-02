
#include "stdafx.h"
#include "Util.h"
#include <math.h>
#include <time.h>
#include <io.h>

// CLockUtil
bool CLockUtil::TryLock() 
{
	return (::TryEnterCriticalSection(&m_csHandle) == TRUE); 
}

//////////////////////////////////////////////////////////////////////
// CExceptMsg

CExceptMsg::CExceptMsg(const char * msg)
{
	assert(msg != NULL);

	m_pMsg = msg;
}

CExceptMsg::~CExceptMsg()
{
}

const char * CExceptMsg::GetMsg() const
{
	return m_pMsg;
}

void CharToWideChar(const char * src, wchar_t * dist,SIZE_T size)
{
	assert(size>strlen(src));
	::MultiByteToWideChar(CP_ACP, 0, src, -1, dist, size);
}

void WideCharToChar(const wchar_t * src, char * dist,SIZE_T size)
{
	assert(size>wcslen(src));
	::WideCharToMultiByte(CP_ACP, 0, src, -1, dist, size, NULL, NULL);
}