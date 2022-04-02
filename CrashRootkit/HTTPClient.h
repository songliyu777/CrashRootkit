
//HTTPClient.hÎÄ¼þ

#ifndef HTTPClient_H_
#define HTTPClient_H_


#include <string>
#include <winsock.h>
#include <iostream>
#include <fstream>


#pragma comment(lib,"ws2_32.lib")

using namespace std;


class HTTPClient
{
public:
	HTTPClient(void);
	~HTTPClient(void);

	bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath, string fileName, string localDirectory,string port);
	bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath , string fileName, string port, OUT PBYTE *buff, OUT SIZE_T* bufSize);
};

#endif

bool GetFileBuff(std::string url,OUT PBYTE *buff,OUT SIZE_T *size);


bool DownloadFileA(const char* remoteFile, const char* localFile, bool isBreakContinue);

bool DownloadFileB(const char* remoteFile, const char* localFile);

SIZE_T DownloadFileInBuff(const char* remoteFile, OUT PBYTE *buff, bool isBreakContinue);