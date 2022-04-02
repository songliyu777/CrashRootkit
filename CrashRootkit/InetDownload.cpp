#include "stdafx.h"
#include "InetDownload.h"
#include "AutoMem.h"
#include <process.h>
#include <assert.h>
#include <string>
#include "Log.h"

using namespace std;

// 写文件
static bool write_file(HANDLE fp, const void* pdata, unsigned int size)
{
	DWORD write_bytes;

	if (!WriteFile(fp, pdata, size, &write_bytes, NULL))
	{
		return false;
	}

	return (write_bytes == size);
}

// InetDownload

void __cdecl InetDownload::WorkerProc(void* lpParameter)
{
	InetDownload* pthis = (InetDownload*)lpParameter;

	while (!pthis->m_bQuit)
	{
		pthis->WorkerFunc();

		Sleep(10);
	}
}

InetDownload::InetDownload()
{
	m_hInternetFile = NULL;

	m_hSession = NULL;
	m_hConnect = NULL;
	m_hRequest = NULL;
	
	m_hFile = INVALID_HANDLE_VALUE;

	m_hThread = NULL;
	m_nState = 0;
	m_bQuit = false;
	m_pFileData = NULL;
	m_nFileSize = 0;
	m_nReadSize = 0;
	m_nPreReadSize = 0;
	m_bBreakAndContinue = false;

	m_strFileUrl = "";
	m_strLocalFile = "";
}

InetDownload::~InetDownload()
{
	Stop();

	if (m_pFileData)
	{
		delete[] m_pFileData;
	}
}

bool InetDownload::Init()
{
	return true;
}

bool InetDownload::Shut()
{
	Stop();

	return true;
}

void InetDownload::SetSession(HINTERNET handle)
{
	m_hSession = handle;
}

const char* InetDownload::GetState() const
{
	switch (m_nState)
	{
	case STATE_BEGIN:
		return "begin";
	case STATE_OPENING:
		return "opening";
	case STATE_OPENED:
		return "opened";
	case STATE_FINISH:
		return "finish";
	case STATE_FAILED:
		return "failed";
	default:
		assert(0);
		break;
	}

	return "";
}

int InetDownload::GetTotalSize() const
{
	return m_nFileSize;
}

int InetDownload::GetReadSize() const
{
	return m_nReadSize;
}

int InetDownload::GetPreReadSize() const
{
	return m_nPreReadSize;
}

void InetDownload::SetFileUrl(const char* value)
{
	assert(value != NULL);

	m_strFileUrl = value;
}

const char* InetDownload::GetFileUrl() const
{
	return m_strFileUrl.c_str();
}

void InetDownload::SetLocalFile(const char* value)
{
	assert(value != NULL);

	m_strLocalFile = value;
}

const char* InetDownload::GetLocalFile() const
{
	return m_strLocalFile.c_str();
}

void InetDownload::SetBreakAndContinue(bool value)
{
	m_bBreakAndContinue = value;
}

bool InetDownload::GetBreakAndContinue() const
{
	return m_bBreakAndContinue;
}

// 开始下载
bool InetDownload::Start()
{
	if (NULL == m_hThread)
	{
		m_nState = STATE_BEGIN;
		m_bQuit = false;
		m_hThread = (HANDLE)_beginthread(WorkerProc, 0, this);
	}

	return true;
}

// 等待线程结束
static bool wait_thread_exit(HANDLE handle)
{
	for (;;)
	{
		DWORD res = WaitForSingleObject(handle, 1000);

		if (res == WAIT_OBJECT_0)
		{
			return true;
		}
		else if (res == WAIT_TIMEOUT)
		{
			DWORD exit_code;

			if (GetExitCodeThread(handle, &exit_code) == FALSE)
			{
				return false;
			}

			if (exit_code != STILL_ACTIVE)
			{
				break;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

// 停止下载
bool InetDownload::Stop()
{
	if (m_hThread)
	{
		m_bQuit = true;

		if (m_nState == STATE_OPENING)
		{
			TerminateThread(m_hThread, 0);
		}
		else
		{
			wait_thread_exit(m_hThread);
		}

		m_hThread = NULL;
	}

	return true;
}

// 下载结束
bool InetDownload::Abort()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	if (m_hRequest != NULL)
	{
		InternetCloseHandle(m_hRequest);
		m_hRequest = NULL;
	}

	if (m_hConnect != NULL)
	{
		InternetCloseHandle(m_hConnect);
		m_hConnect = NULL;
	}
	
	return true;
}

bool InetDownload::WorkerFunc()
{
	if (m_nState != STATE_BEGIN)
	{
		return false;
	}
	//修改超时问题，不需要断点续传
	if (!WorkerFuncBreakAndContinue())
	{
		//m_nState = STATE_FAILED;
		return WorkerFuncNoBreakAndContinue();
		//return false;
	}

	return true; 
}

typedef struct _PARM
{
	HINTERNET hSession;
	string host;
}PARM,PPARM;

HINTERNET g_hInternetFile = 0;

DWORD WINAPI WorkerFunctionOpenUrl(IN LPVOID vThreadParm)
{
	PARM* pThreadParm;
	// Initialize local pointer to void pointer passed to thread
	pThreadParm = (PARM*)vThreadParm;
	g_hInternetFile = InternetOpenUrlA(pThreadParm->hSession, pThreadParm->host.c_str(),
		NULL, 0, INTERNET_FLAG_RAW_DATA, 0);

	if (NULL == g_hInternetFile)
	{
		PrintLog(L"WorkerFunctionOpenUrl failed\r\n");
		return false;
	}

	return 0;  // success
}

bool InetDownload::WorkerFuncNoBreakAndContinue()
{
	if (m_pFileData)
	{
		delete[] m_pFileData;
		m_pFileData = NULL;
	}

	m_nFileSize = 0;
	m_nReadSize = 0;
	m_nPreReadSize = 0;
	memset(&m_FileTime, 0, sizeof(m_FileTime));
	m_nState = STATE_OPENING;
	
	// INTERNET_FLAG_RAW_DATA			真正的断点续传
	// INTERNET_FLAG_EXISTING_CONNECT	不是真正的断点续传
	// INTERNET_FLAG_RELOAD				不是真正的断点续传

	PARM parm;
	parm.hSession = m_hSession;
	parm.host = m_strFileUrl;

	DWORD dwThreadID;

	HANDLE hThread = CreateThread(
		NULL,            // Pointer to thread security attributes 
		0,               // Initial thread stack size, in bytes 
		WorkerFunctionOpenUrl,  // Pointer to thread function 
		&parm,     // The argument for the new thread
		0,               // Creation flags 
		&dwThreadID      // Pointer to returned thread identifier 
		);    

	// Wait for the call to InternetConnect in worker function to complete
	DWORD dwTimeout = 6000; // in milliseconds
	if (WaitForSingleObject ( hThread, dwTimeout ) == WAIT_TIMEOUT )
	{
		TerminateThread(hThread,0);
		m_nState = STATE_FAILED;
		return false;
	}

	m_hInternetFile = g_hInternetFile;
	//m_hInternetFile = InternetOpenUrlA(m_hSession, m_strFileUrl.c_str(),
	//	NULL, 0, INTERNET_FLAG_RAW_DATA, 0);

	if (NULL == m_hInternetFile)
	{
		m_nState = STATE_FAILED;
		return false;
	}

	m_nState = STATE_OPENED;

	DWORD length;
	DWORD readed = sizeof(length);

	// 获得文件长度
	BOOL succeed = HttpQueryInfo(m_hInternetFile, 
		HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &length, &readed, 
		NULL);

	if (!succeed)
	{
		InternetCloseHandle(m_hInternetFile);
		m_nState = STATE_FAILED;
		return false;
	}

	SYSTEMTIME file_st;

	readed = sizeof(file_st);

	succeed = HttpQueryInfo(m_hInternetFile,
		HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME, &file_st, 
		&readed, NULL);

	if (!succeed)
	{
		InternetCloseHandle(m_hInternetFile);
		m_nState = STATE_FAILED;
		return false;
	}

	// 是否要延迟写入文件
	bool delay_write = m_strLocalFile.empty();

	m_nFileSize = length;
	m_FileTime = file_st;
	m_FileTime.wMilliseconds = 500;

	HANDLE fp = INVALID_HANDLE_VALUE;

	if (delay_write)
	{
		// 下载到内存中
		m_pFileData = new char[length];

		if (NULL == m_pFileData)
		{
			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
			return false;
		}
	}

	// 是否要断点续传
	if (m_bBreakAndContinue)
	{
		// 读取已经下载的文件长度
		HANDLE read_fp = CreateFileA(m_strLocalFile.c_str(), GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 
			NULL);

		if (read_fp != INVALID_HANDLE_VALUE)
		{
			m_nPreReadSize = ::GetFileSize(read_fp, NULL);

			if (delay_write)
			{
				ReadFile(read_fp, m_pFileData, m_nPreReadSize, &readed, NULL);
			}

			CloseHandle(read_fp);
		}
	}

	// 直接写入文件
	if (!delay_write)
	{
		// 断点续传时、打开存在的文件
		if (m_bBreakAndContinue)
		{
			fp = CreateFileA(m_strLocalFile.c_str(), GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
		}
		// 非断点续传时、创建新的文件
		else
		{
			fp = CreateFileA(m_strLocalFile.c_str(), GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
		}

		if (INVALID_HANDLE_VALUE == fp)
		{
			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
			return false;
		}
	}

	if (m_nPreReadSize > 0)
	{
		// 从上次结束的地方开始下载
		DWORD dwRet = InternetSetFilePointer(m_hInternetFile, m_nPreReadSize, NULL, 
			FILE_BEGIN, 0);

		if (dwRet == -1)
		{
			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
			return false;
		}

		// 从上次结束的地方开始写入
		dwRet = SetFilePointer(fp, m_nPreReadSize, NULL, FILE_BEGIN);

		if (dwRet == -1)
		{
			if (fp != INVALID_HANDLE_VALUE)
			{
				CloseHandle(fp);
			}

			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
			return false;
		}
	}

	TAutoMem<char, 1> auto_buf(READ_BUFFER_LEN);
	char* buffer = auto_buf.GetBuffer();

	while ((m_nPreReadSize + m_nReadSize < length) && (!m_bQuit))
	{
		succeed = InternetReadFile(m_hInternetFile, buffer, 
			READ_BUFFER_LEN, &readed);

		if (!succeed)
		{
			if (fp != INVALID_HANDLE_VALUE)
			{
				CloseHandle(fp);
			}

			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
			return false;
		}

		if ((m_nPreReadSize + m_nReadSize + readed) > length)
		{
			if (fp != INVALID_HANDLE_VALUE)
			{
				CloseHandle(fp);
			}

			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
		}

		if (readed == 0)
		{
			// 文件读取结束
			break;
		}

		if (delay_write)
		{
			memcpy(m_pFileData + m_nPreReadSize + m_nReadSize, buffer, readed);
		}
		else
		{
			if (!write_file(fp, buffer, readed))
			{
				if (fp != INVALID_HANDLE_VALUE)
				{
					CloseHandle(fp);
				}

				InternetCloseHandle(m_hInternetFile);
				m_nState = STATE_FAILED;
				return false;
			}
		}

		m_nReadSize += readed;
	}

	if (fp != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fp);
	}

	InternetCloseHandle(m_hInternetFile);

	m_hInternetFile = NULL;
	m_nState = STATE_FINISH;

	return true;
}

HINTERNET g_hConnect = 0;

/////////////////// WorkerFunction ////////////////////// 
DWORD WINAPI WorkerFunctionConnect(IN LPVOID vThreadParm)
{
	PARM* pThreadParm;
	// Initialize local pointer to void pointer passed to thread
	pThreadParm = (PARM*)vThreadParm;
	g_hConnect = 0;
	g_hConnect = InternetConnectA(pThreadParm->hSession, pThreadParm->host.c_str(),
		INTERNET_DEFAULT_HTTP_PORT,	NULL, NULL,	INTERNET_SERVICE_HTTP, 0, 0);
	if(!g_hConnect)
	{
		PrintLog(L"WorkerFunctionConnect failed!\r\n");
		return -1;
	}

	return 0;  // success
}

BOOL g_bRequest = false;
DWORD WINAPI WorkerFunctionRequest(IN LPVOID vThreadParm)
{
	g_bRequest = false;
	PARM* pThreadParm;
	// Initialize local pointer to void pointer passed to thread
	pThreadParm = (PARM*)vThreadParm;

	g_bRequest = HttpSendRequest(pThreadParm->hSession, NULL, 0, NULL, 0);

	if(!g_bRequest)
	{
		PrintLog(L"WorkerFunctionRequest failed\r\n");
		return -1;
	}

	return 0;  // success
}

// 文件下载
bool InetDownload::WorkerFuncBreakAndContinue()
{
	if (m_pFileData)
	{
		delete[] m_pFileData;
		m_pFileData = NULL;
	}

	m_nState = STATE_OPENING;

	m_nFileSize = 0;
	m_nReadSize = 0;
	m_nPreReadSize = 0;
	memset(&m_FileTime, 0, sizeof(m_FileTime));
	
	// ①取得本地文件的大小
	if (m_bBreakAndContinue)
	{
		HANDLE read_fp = CreateFileA(m_strLocalFile.c_str(), GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (read_fp != INVALID_HANDLE_VALUE)
		{
			m_nPreReadSize = ::GetFileSize(read_fp, NULL);

			CloseHandle(read_fp);
		}
	}

	// ②建立连接
	std::string host = "";
	std::string file = "";
	ParseURL(m_strFileUrl.c_str(), host, file);
	
	PARM parm;
	parm.hSession = m_hSession;
	parm.host = host;

	DWORD dwThreadID;

	/*m_hConnect = InternetConnectA(m_hSession, host.c_str(),
		INTERNET_DEFAULT_HTTP_PORT,	NULL, NULL,	INTERNET_SERVICE_HTTP, 0, 0);*/

	//修改连接延时控制
	HANDLE hThread = CreateThread(
		NULL,            // Pointer to thread security attributes 
		0,               // Initial thread stack size, in bytes 
		WorkerFunctionConnect,  // Pointer to thread function 
		&parm,     // The argument for the new thread
		0,               // Creation flags 
		&dwThreadID      // Pointer to returned thread identifier 
		);    

	// Wait for the call to InternetConnect in worker function to complete
	DWORD dwTimeout = 6000; // in milliseconds
	if (WaitForSingleObject ( hThread, dwTimeout ) == WAIT_TIMEOUT )
	{
		TerminateThread(hThread,0);
		return false;
	}
	m_hConnect = g_hConnect;
	CloseHandle (hThread);


	if (m_hConnect == NULL)
	{
		Abort();
		//m_nState = STATE_FAILED;
		return false;
	}
	// ③初始化下载请求
	const char* FAcceptTypes = "*/*";
	
	m_hRequest = HttpOpenRequestA(m_hConnect, "GET", file.c_str(),
		HTTP_VERSIONA, NULL, &FAcceptTypes, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, 0);


	if (m_hRequest == NULL)
	{
		m_hRequest = HttpOpenRequestA(m_hConnect, "GET", file.c_str(),
			"HTTP/1.0", NULL, &FAcceptTypes, INTERNET_FLAG_RELOAD, 0);
	}

	if (m_hRequest == NULL)
	{
		m_hRequest = HttpOpenRequestA(m_hConnect, "GET", file.c_str(),
			NULL, NULL, &FAcceptTypes, INTERNET_FLAG_RELOAD, 0);
	}
	
	if (m_hRequest == NULL)
	{
		Abort();
		//m_nState = STATE_FAILED;
		return false;
	}

	// ④添加请求头信息
	if (m_nPreReadSize > 0)
	{
		char headBuf[256];
		memset(headBuf, 0, 256);
		sprintf(headBuf, "Range:bytes=%d-\r\n", m_nPreReadSize);

		BOOL bSucceed = HttpAddRequestHeadersA(m_hRequest, headBuf, -1,
			HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);

		if (bSucceed == FALSE)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}
	}

	// ⑤发送下载请求
	//BOOL bSucceed = HttpSendRequest(m_hRequest, NULL, 0, NULL, 0);
	//修改请求超时
	BOOL bSucceed;
	parm.hSession = m_hRequest;
	hThread = CreateThread(
		NULL,            // Pointer to thread security attributes 
		0,               // Initial thread stack size, in bytes 
		WorkerFunctionRequest,  // Pointer to thread function 
		&parm,     // The argument for the new thread
		0,               // Creation flags 
		&dwThreadID      // Pointer to returned thread identifier 
		);    

	// Wait for the call to InternetConnect in worker function to complete
	dwTimeout = 6000; // in milliseconds
	if (WaitForSingleObject ( hThread, dwTimeout ) == WAIT_TIMEOUT )
	{
		TerminateThread(hThread,0);
	}
	CloseHandle(hThread);
	bSucceed = g_bRequest;
	if (bSucceed == FALSE)
	{
		Abort();
		//m_nState = STATE_FAILED;
		return false;
	}
	
	// ⑥获得文件长度
	DWORD size_readed = sizeof(m_nFileSize);
	
	bSucceed = HttpQueryInfo(m_hRequest, 
		HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &m_nFileSize, &size_readed, NULL);

	if (bSucceed == FALSE)
	{
		Abort();
		//m_nState = STATE_FAILED;
		return false;
	}

	// 取得的大小只是剩余大小
	m_nFileSize += m_nPreReadSize;

	// ⑦获得文件时间
	DWORD time_readed = sizeof(m_FileTime);

	bSucceed = HttpQueryInfo(m_hRequest,
		HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME, &m_FileTime, &time_readed, NULL);

	if (bSucceed == FALSE)
	{
		Abort();
		//m_nState = STATE_FAILED;
		return false;
	}
	m_FileTime.wMilliseconds = 500;

	m_nState = STATE_OPENED;
	
	bool delay_write = m_strLocalFile.empty();

	// ⑧延迟写入时
	if (delay_write)
	{
		// 申请内存空间
		m_pFileData = new char[m_nFileSize];

		if (NULL == m_pFileData)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}

		// 断点续传时
		if (m_nPreReadSize > 0)
		{
			HANDLE read_fp = CreateFileA(m_strLocalFile.c_str(), GENERIC_READ, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			
			if (read_fp != INVALID_HANDLE_VALUE)
			{
				DWORD data_readed = m_nPreReadSize;

				ReadFile(read_fp, m_pFileData, m_nPreReadSize, &data_readed, NULL);

				CloseHandle(read_fp);
			}
		}
	}

	// ⑨直接写入文件
	if (!delay_write)
	{
		// 断点续传时、打开存在的文件
		if (m_bBreakAndContinue)
		{
			m_hFile = CreateFileA(m_strLocalFile.c_str(), GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
		}
		// 非断点续传时、创建新的文件
		else
		{
			m_hFile = CreateFileA(m_strLocalFile.c_str(), GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
		}

		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}
	}
	
	// ⑩从上次结束的地方开始写入
	if (m_nPreReadSize > 0)
	{
		DWORD dwRet = SetFilePointer(m_hFile, m_nPreReadSize, NULL, FILE_BEGIN);

		if (dwRet == -1)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}
	}

	// 开始下载文件
	TAutoMem<char, 1> auto_buf(READ_BUFFER_LEN);
	char* buffer = auto_buf.GetBuffer();
	DWORD temp_readed = 0;

	while ((m_nPreReadSize + m_nReadSize < m_nFileSize) && (!m_bQuit))
	{
		// 读取远程文件
		bSucceed = InternetReadFile(m_hRequest, buffer, 
			READ_BUFFER_LEN, &temp_readed);

		if (bSucceed == FALSE)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}

		// 是否读取正确
		if ((m_nPreReadSize + m_nReadSize + temp_readed) > m_nFileSize)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}

		// 文件读取结束
		if (temp_readed == 0)
		{
			break;
		}

		// 读取的内容保存到文件或者内存
		if (delay_write)
		{
			memcpy(m_pFileData + m_nPreReadSize + m_nReadSize, buffer, temp_readed);
		}
		else
		{
			if (!write_file(m_hFile, buffer, temp_readed))
			{
				Abort();
				//m_nState = STATE_FAILED;
				return false;
			}
		}

		m_nReadSize += temp_readed;
	}

	Abort();
	m_nState = STATE_FINISH;

	return true;
}


char* InetDownload::GetFileData()
{
	return m_pFileData;
}

DWORD InetDownload::GetFileSize()
{
	return m_nFileSize;
}

// 将下载到的内容写入文件
bool InetDownload::WriteToFile(const char* file_name)
{
	assert(file_name != NULL);

	if (m_nState != STATE_FINISH)
	{
		return false;
	}

	if (NULL == m_pFileData)
	{
		return false;
	}

	HANDLE fp = CreateFileA(file_name, GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);

	if (INVALID_HANDLE_VALUE == fp)
	{
		return false;
	}

	if (!write_file(fp, m_pFileData, m_nFileSize))
	{
		CloseHandle(fp);
		return false;
	}

	// 写入文件时间
	FILETIME ft;

	SystemTimeToFileTime(&m_FileTime, &ft);

	if (!SetFileTime(fp, &ft, &ft, &ft))
	{
		CloseHandle(fp);
		return false;
	}

	CloseHandle(fp);

	return true;
}

