//--------------------------------------------------------------------
// 文件名:		InetDownload.h
// 内  容:		
// 说  明:		
// 创建日期:	2009年6月13日
// 创建人:		陆利民
// 版权所有:	苏州蜗牛电子有限公司
//--------------------------------------------------------------------

#ifndef _INETDOWNLOAD_H
#define _INETDOWNLOAD_H

#include <string>
#include <windows.h>
#include <wininet.h>

// 文件下载

class InetDownload
{
private:
	enum { READ_BUFFER_LEN = 8192 };

	enum 
	{
		STATE_BEGIN,
		STATE_OPENING,
		STATE_OPENED,
		STATE_FINISH,
		STATE_FAILED,
	};
	
	static void __cdecl WorkerProc(void* lpParameter);

public:
	InetDownload();
	virtual ~InetDownload();

	virtual bool Init();
	virtual bool Shut();
	
	// 设置网络句柄
	void SetSession(HINTERNET handle);

	// 获得当前下载状态
	const char* GetState() const;
	// 获得文件长度
	int GetTotalSize() const;
	// 获得已下载的长度
	int GetReadSize() const;
	// 断点续传时上次下载的长度
	int GetPreReadSize() const;

	// 文件URL
	void SetFileUrl(const char* value);
	const char* GetFileUrl() const;

	// 保存到本地的文件名
	void SetLocalFile(const char* value);
	const char* GetLocalFile() const;

	// 是否要断点续传
	void SetBreakAndContinue(bool value);
	bool GetBreakAndContinue() const;

	// 开始下载
	bool Start();
	// 停止下载
	bool Stop();
	// 将下载到的内容写入文件
	bool WriteToFile(const char* file_name);

	char* GetFileData();

	DWORD GetFileSize();
private:
	// 文件下载处理
	bool WorkerFunc();

	bool WorkerFuncNoBreakAndContinue();
	
	bool WorkerFuncBreakAndContinue();

	// 下载结束
	bool Abort();

	// 从URL中提取主机名称和下载文件路径
	void ParseURL(const char* url, std::string& HostName, std::string& FileName)
	{
		std::string strUrl = url;
		std::string strTmp = strUrl.substr(0, 7);

		if (strTmp.compare("http://") == 0)
		{
			strUrl = strUrl.substr(7, strUrl.length());
		}

		int i = strUrl.find("/");
		HostName = strUrl.substr(0, i);
		FileName = strUrl.substr(i+1, strUrl.length());
	}

private:
	HINTERNET m_hInternetFile;

	HINTERNET m_hSession;
	HINTERNET m_hConnect;
	HINTERNET m_hRequest;
	
	HANDLE m_hFile;

	HANDLE m_hThread;
	bool m_bQuit;
	int m_nState;
	
	char* m_pFileData;
	DWORD m_nFileSize;
	DWORD m_nReadSize;
	DWORD m_nPreReadSize;
	SYSTEMTIME m_FileTime;

	std::string m_strFileUrl;
	std::string m_strLocalFile;
	
	bool m_bBreakAndContinue;
};

#endif // _INETDOWNLOAD_H

