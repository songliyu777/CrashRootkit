#include "stdafx.h"
#include "InetDownload.h"
#include "AutoMem.h"
#include <process.h>
#include <assert.h>
#include <string>
#include "Log.h"

using namespace std;

// д�ļ�
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

// ��ʼ����
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

// �ȴ��߳̽���
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

// ֹͣ����
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

// ���ؽ���
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
	//�޸ĳ�ʱ���⣬����Ҫ�ϵ�����
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
	
	// INTERNET_FLAG_RAW_DATA			�����Ķϵ�����
	// INTERNET_FLAG_EXISTING_CONNECT	���������Ķϵ�����
	// INTERNET_FLAG_RELOAD				���������Ķϵ�����

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

	// ����ļ�����
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

	// �Ƿ�Ҫ�ӳ�д���ļ�
	bool delay_write = m_strLocalFile.empty();

	m_nFileSize = length;
	m_FileTime = file_st;
	m_FileTime.wMilliseconds = 500;

	HANDLE fp = INVALID_HANDLE_VALUE;

	if (delay_write)
	{
		// ���ص��ڴ���
		m_pFileData = new char[length];

		if (NULL == m_pFileData)
		{
			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
			return false;
		}
	}

	// �Ƿ�Ҫ�ϵ�����
	if (m_bBreakAndContinue)
	{
		// ��ȡ�Ѿ����ص��ļ�����
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

	// ֱ��д���ļ�
	if (!delay_write)
	{
		// �ϵ�����ʱ���򿪴��ڵ��ļ�
		if (m_bBreakAndContinue)
		{
			fp = CreateFileA(m_strLocalFile.c_str(), GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
		}
		// �Ƕϵ�����ʱ�������µ��ļ�
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
		// ���ϴν����ĵط���ʼ����
		DWORD dwRet = InternetSetFilePointer(m_hInternetFile, m_nPreReadSize, NULL, 
			FILE_BEGIN, 0);

		if (dwRet == -1)
		{
			InternetCloseHandle(m_hInternetFile);
			m_nState = STATE_FAILED;
			return false;
		}

		// ���ϴν����ĵط���ʼд��
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
			// �ļ���ȡ����
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

// �ļ�����
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
	
	// ��ȡ�ñ����ļ��Ĵ�С
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

	// �ڽ�������
	std::string host = "";
	std::string file = "";
	ParseURL(m_strFileUrl.c_str(), host, file);
	
	PARM parm;
	parm.hSession = m_hSession;
	parm.host = host;

	DWORD dwThreadID;

	/*m_hConnect = InternetConnectA(m_hSession, host.c_str(),
		INTERNET_DEFAULT_HTTP_PORT,	NULL, NULL,	INTERNET_SERVICE_HTTP, 0, 0);*/

	//�޸�������ʱ����
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
	// �۳�ʼ����������
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

	// ���������ͷ��Ϣ
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

	// �ݷ�����������
	//BOOL bSucceed = HttpSendRequest(m_hRequest, NULL, 0, NULL, 0);
	//�޸�����ʱ
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
	
	// �޻���ļ�����
	DWORD size_readed = sizeof(m_nFileSize);
	
	bSucceed = HttpQueryInfo(m_hRequest, 
		HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &m_nFileSize, &size_readed, NULL);

	if (bSucceed == FALSE)
	{
		Abort();
		//m_nState = STATE_FAILED;
		return false;
	}

	// ȡ�õĴ�Сֻ��ʣ���С
	m_nFileSize += m_nPreReadSize;

	// �߻���ļ�ʱ��
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

	// ���ӳ�д��ʱ
	if (delay_write)
	{
		// �����ڴ�ռ�
		m_pFileData = new char[m_nFileSize];

		if (NULL == m_pFileData)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}

		// �ϵ�����ʱ
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

	// ��ֱ��д���ļ�
	if (!delay_write)
	{
		// �ϵ�����ʱ���򿪴��ڵ��ļ�
		if (m_bBreakAndContinue)
		{
			m_hFile = CreateFileA(m_strLocalFile.c_str(), GENERIC_WRITE, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
		}
		// �Ƕϵ�����ʱ�������µ��ļ�
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
	
	// ����ϴν����ĵط���ʼд��
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

	// ��ʼ�����ļ�
	TAutoMem<char, 1> auto_buf(READ_BUFFER_LEN);
	char* buffer = auto_buf.GetBuffer();
	DWORD temp_readed = 0;

	while ((m_nPreReadSize + m_nReadSize < m_nFileSize) && (!m_bQuit))
	{
		// ��ȡԶ���ļ�
		bSucceed = InternetReadFile(m_hRequest, buffer, 
			READ_BUFFER_LEN, &temp_readed);

		if (bSucceed == FALSE)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}

		// �Ƿ��ȡ��ȷ
		if ((m_nPreReadSize + m_nReadSize + temp_readed) > m_nFileSize)
		{
			Abort();
			//m_nState = STATE_FAILED;
			return false;
		}

		// �ļ���ȡ����
		if (temp_readed == 0)
		{
			break;
		}

		// ��ȡ�����ݱ��浽�ļ������ڴ�
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

// �����ص�������д���ļ�
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

	// д���ļ�ʱ��
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

