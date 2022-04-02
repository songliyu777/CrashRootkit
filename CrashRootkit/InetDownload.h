//--------------------------------------------------------------------
// �ļ���:		InetDownload.h
// ��  ��:		
// ˵  ��:		
// ��������:	2009��6��13��
// ������:		½����
// ��Ȩ����:	������ţ�������޹�˾
//--------------------------------------------------------------------

#ifndef _INETDOWNLOAD_H
#define _INETDOWNLOAD_H

#include <string>
#include <windows.h>
#include <wininet.h>

// �ļ�����

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
	
	// ����������
	void SetSession(HINTERNET handle);

	// ��õ�ǰ����״̬
	const char* GetState() const;
	// ����ļ�����
	int GetTotalSize() const;
	// ��������صĳ���
	int GetReadSize() const;
	// �ϵ�����ʱ�ϴ����صĳ���
	int GetPreReadSize() const;

	// �ļ�URL
	void SetFileUrl(const char* value);
	const char* GetFileUrl() const;

	// ���浽���ص��ļ���
	void SetLocalFile(const char* value);
	const char* GetLocalFile() const;

	// �Ƿ�Ҫ�ϵ�����
	void SetBreakAndContinue(bool value);
	bool GetBreakAndContinue() const;

	// ��ʼ����
	bool Start();
	// ֹͣ����
	bool Stop();
	// �����ص�������д���ļ�
	bool WriteToFile(const char* file_name);

	char* GetFileData();

	DWORD GetFileSize();
private:
	// �ļ����ش���
	bool WorkerFunc();

	bool WorkerFuncNoBreakAndContinue();
	
	bool WorkerFuncBreakAndContinue();

	// ���ؽ���
	bool Abort();

	// ��URL����ȡ�������ƺ������ļ�·��
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

