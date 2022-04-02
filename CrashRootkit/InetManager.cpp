#include "stdafx.h"
#include "InetManager.h"
#include "InetDownload.h"
#include "AutoMem.h"
#include <assert.h>

#pragma comment(lib, "wininet.lib")

// InetManager

InetManager::InetManager()
{
	m_hSession = NULL;
}

InetManager::~InetManager()
{
	if (m_hSession)
	{
		InternetCloseHandle(m_hSession);
		m_hSession = NULL;
	}
}

bool InetManager::Init()
{
	m_hSession = InternetOpen(L"Mozilla/4.0 (compatible)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	
	if (NULL == m_hSession)
	{
		DWORD error = GetLastError();
		return false;
	}
	
	return true;
}

bool InetManager::Shut()
{
	if (m_hSession)
	{
		InternetCloseHandle(m_hSession);
		m_hSession = NULL;
	}
	
	return true;
}

void* InetManager::CreateDownload()
{
	InetDownload* pDownload = new InetDownload();
	
	if (pDownload)
		pDownload->SetSession(m_hSession);
	
	return (void*)(pDownload);
}

bool InetManager::DeleteDownload(void *pVoid)
{
	if (NULL == pVoid)
	{
		return false;
	}

	InetDownload* pDownload = (InetDownload*)pVoid;

	delete pDownload;
	pDownload = NULL;

	return true;
}

// 是否包含本地编码字符
static bool has_local_char(const char* str)
{
	for (const char* s = str; *s; ++s)
	{
		if (unsigned char(*s) > 127)
		{
			return true;
		}
	}

	return false;
}

// 转换到宽字符串
const wchar_t * ToWideStr(const char * info, 
						  wchar_t * buf, size_t size)
{
	const size_t len = size / sizeof(wchar_t);
	size_t res = ::MultiByteToWideChar(CP_ACP, 0, info, -1, buf, int(len));

	if (res == 0)
	{
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			buf[len - 1] = 0;
		}
		else
		{
			buf[0] = 0;
		}
	}
	return buf;
}

int GetToWideStrLen(const char* str)
{
	int size = ::MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);

	return size;
}
// 本地编码转换为宽字符串
static std::wstring local_to_utf16(const char* str)
{
	assert(str != NULL);

	size_t size = GetToWideStrLen(str);

	TAutoMem<wchar_t, 256> buf(size);

	wchar_t* p = buf.GetBuffer();

	ToWideStr(str, p, size * sizeof(wchar_t));

	return std::wstring(p);
}

// 宽字符串转换为UTF8
static std::string utf16_to_utf8(const wchar_t* str)
{
	// 获得长度
	int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);

	TAutoMem<char, 256> auto_buf(size);

	char* buf = auto_buf.GetBuffer();

	int res = WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, size, NULL, NULL);

	if (0 == res)
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			buf[size - 1] = 0;
		}
		else
		{
			buf[0] = 0;
		}
	}

	return std::string(buf);
}

std::string InetManager::CheckUrl(const char* url)
{
	assert(url != NULL);

	bool need_convert = has_local_char(url);
	
	if ((strchr(url, '\\') == NULL) && (!need_convert))
	{
		return std::string(url);
	}

	std::string utf8_str;
	
	if (need_convert)
	{
		// 包含本地编码的字符串要转换成UTF8
		std::wstring ws = local_to_utf16(url);

		utf8_str = utf16_to_utf8(ws.c_str());

		url = utf8_str.c_str();
	}

	char to_hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
		'A', 'B', 'C', 'D', 'E', 'F', };

	size_t size = strlen(url) + 1;

	TAutoMem<char, 256> auto_buf(size * 3);

	char* p = auto_buf.GetBuffer();

	for (const char* s = url; *s; ++s)
	{
		if (*s == '\\')
		{
			*p++ = '/';
		}
		else if (unsigned char(*s) > 0x7F)
		{
			*p++ = '%';
			*p++ = to_hex[(unsigned char(*s) >> 4) & 0xF];
			*p++ = to_hex[unsigned char(*s) & 0xF];
		}
		else
		{
			*p++ = *s;
		}
	}

	*p = 0;

	return std::string(auto_buf.GetBuffer());
}
