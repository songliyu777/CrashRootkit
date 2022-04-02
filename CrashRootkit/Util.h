
#ifndef _UTIL_H
#define _UTIL_H

//#pragma warning(disable:4786)

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <WTypes.h>
#include <time.h>
#include <wincon.h>
#include "string"
#include "vector"
using namespace std;

// CLockUtil
// 共享资源互斥访问锁
class CLockUtil
{
public:
	CLockUtil()
	{
		::InitializeCriticalSection(&m_csHandle);
	}
	~CLockUtil()
	{
		::DeleteCriticalSection(&m_csHandle);
	}
	// 锁定
	void Lock()
	{
		::EnterCriticalSection(&m_csHandle);
	}
	// 释放
	void Unlock()
	{
		::LeaveCriticalSection(&m_csHandle);
	}
	bool TryLock();

private:
	CRITICAL_SECTION	m_csHandle;
};

// CAutoLock
// 用结构析构函数自动加解锁
class CAutoLock
{
public:
	CAutoLock(CLockUtil * plock)
	{
		assert(plock != NULL);
		m_pLock = plock;
		m_pLock->Lock();
	}
	~CAutoLock()
	{
		m_pLock->Unlock();
	}

private:
	CLockUtil *		m_pLock;
};

// CLockVal
class CLockVal
{
public:
	CLockVal()
	{
		m_nVal = 0;
	}
	int Inc()
	{
		return ::InterlockedIncrement(&m_nVal);
	}
	int Dec()
	{
		return ::InterlockedDecrement(&m_nVal);
	}
	void Set(int val)
	{
		::InterlockedExchange(&m_nVal, val);
	}
	int Get() const
	{
		return m_nVal;
	}

private:
	long	m_nVal;
};

// CExceptMsg
class CExceptMsg
{
public:
	CExceptMsg();

	CExceptMsg(const char * msg);

	~CExceptMsg();

	const char * GetMsg() const;

private:
	const char * m_pMsg;
};


void CharToWideChar(const char * src, wchar_t * dist,SIZE_T size);

void WideCharToChar(const wchar_t * src, char * dist,SIZE_T size);


#endif // _UTIL_H
