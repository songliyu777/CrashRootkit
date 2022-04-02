//HTTPClient.cpp�ļ�
#include "stdafx.h"
#include "HTTPClient.h"
#include "Log.h"
#include "InetDownload.h"
#include "InetManager.h"

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <string>
#include "Wininet.h"
#pragma comment(lib,"Wininet.lib")


HTTPClient::HTTPClient(void)
{
}

HTTPClient::~HTTPClient(void)
{
}

bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath, string fileName, string localDirectory,string port)
{
	ofstream fout;
	string newfile = localDirectory + fileName;


	//ʶ������·�����ļ���
	std::locale::global(std::locale(""));

	fout.open(newfile.c_str(),ios_base::binary);
	if(!fout.is_open())
	{
		PrintDbgString(L"�������ļ���ʧ�� \r\n");
		return false;
	}

	PBYTE buff;
	SIZE_T bufSize;
	if(DownloadFile(parmSock,serverName,sourcePath,fileName,port,&buff,&bufSize))
	{
		fout.write((const char *)buff,bufSize);
		fout.close();
		return true;
	}
	fout.close();
	return false; 
}  

bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath , string fileName, string port, OUT PBYTE *buff, OUT SIZE_T* bufSize)
{
	//ʶ������·�����ļ���
	std::locale::global(std::locale(""));

	char szBuffer[1024];
	//��Ŀ¼���ļ����е�' 'ת��Ϊ"%20"
	//sourcePath
	int npos = sourcePath.find(" ");
	while(npos + 1)
	{
		npos = sourcePath.find(" ");
		if(npos == -1)
			break;
		sourcePath.replace(npos,strlen(" "),"%20");
	}

	//fileName   
		npos = fileName.find(" ");  
		while(npos + 1)  
		{  
			npos = fileName.find(" ");  
			if(npos == -1)  
				break;  
			fileName.replace(npos,strlen(" "),"%20");  
		} 

	string requestFile = sourcePath + fileName;
	///////////////////////////////////////////////////////////////
	//��ʽ��httpͷ
	//��һ��
	sprintf(szBuffer,"GET %s HTTP/1.1\r\n",requestFile.c_str());

	//�ڶ���
	strcat(szBuffer,"Host:");
	strcat(szBuffer,serverName.c_str());
	strcat(szBuffer,":");
	strcat(szBuffer,port.c_str());
	strcat(szBuffer,"\r\n");

	//������
	//strcat(szBuffer,"Accept:*/*");
	/*strcat(szBuffer,"\r\n");


	//////////////////////////
	//������(���п���)
	strcat(szBuffer,"User-Agent:GeneralDownloadApplication");
	strcat(szBuffer,"\r\n");
	*/

	//������ �ǳ־�����
	strcat(szBuffer,"Connection:close");
	strcat(szBuffer,"\r\n");
	//���һ��
	strcat(szBuffer,"\r\n");

	int nRet = send(parmSock, szBuffer, strlen(szBuffer), 0);
	if (nRet == SOCKET_ERROR)
	{
		PrintDbgString(L"send() error \r\n");
		closesocket(parmSock);  
		WSACleanup();
		return false;
	}

	//
	// Receive the file contents and print to stdout
	//
	///////////////////////////////////////////////////////////////
	//ȡ��httpͷ,��СӦ�ò�����1024���ֽ�,ֻ�ڵ�һ�ν���ʱ���д���
	///////////////////////////////////////////////////////////////
	nRet = recv(parmSock, szBuffer, sizeof(szBuffer), 0);
	if (nRet == SOCKET_ERROR)
	{
		PrintDbgString(L"recv() error \r\n");
		closesocket(parmSock);
		WSACleanup();
		return false;
	}
	////////////////////////////////////////////////////// 

	//�жϸ��ļ��Ƿ����   
	std::string strBuffer = szBuffer;   
	std::string firstString = strBuffer.substr(0,strBuffer.find("\r\n"));   
	int File_Exist = firstString.find("200");   
	if(File_Exist == -1)   
	{    
		PrintDbgString(L"File not found in the web.\r\n");
		WSACleanup();
		return false;      
	}
	int File_Size = strBuffer.find("Content-Length:");
	if(File_Size == -1)
	{
		PrintDbgString(L"Content-Length: not found size in the web.\r\n");
		WSACleanup();
		return false;   
	}
	//��ȡ�ļ����ݿ�ʼλ�ú��ļ���С
	int start = File_Size + 0x10;
	int end = strBuffer.find("\r\n\r\n",File_Size);
	int size = end - start;
	std::string fsizeString = strBuffer.substr(start,size);
	int i = 0; 
	long filesize = atol(fsizeString.c_str());
	*bufSize = filesize;
	*buff = (PBYTE)malloc(filesize);
	if(!*buff)
	{
		closesocket(parmSock);  
		WSACleanup();
		return false;
	}
	//�ļ����ݿ�ʼ
	int fileBegin = end + 4;
	int fileseek = 0;
	/////////////////////////////////////////////////////////////
	for(i = fileBegin; i < nRet; i++)
	{
		(*buff)[fileseek] = szBuffer[i];
		fileseek++;
	}

	while(1)  
	{  
		// Wait to receive, nRet = NumberOfBytesReceived   
		nRet = recv(parmSock, szBuffer, sizeof(szBuffer), 0);  

		if (nRet == SOCKET_ERROR)  
		{  
			PrintDbgString(L"recv() error.\r\n");
			closesocket(parmSock);  
			WSACleanup();
			free(*buff);
			return false;  
		}  
		/////////////////////////////////////////////////////////////////   
		if (nRet == 0)  
			break;
		memcpy((*buff) + fileseek, szBuffer,nRet);
		fileseek += nRet;
	}  
	closesocket(parmSock);      
	WSACleanup();
	return true; 
}  

//main.cpp�ļ�   

#include "HTTPClient.h" 
//��ȡ�����ļ�
bool GetFileBuff(std::string url,OUT PBYTE *buff,OUT SIZE_T *size)
{
	HTTPClient client;
	std::string port = "80";  

	std::string removeProtocol;  
	std::string serverName;  
	std::string sourcePath;  
	std::string fileName;  
	int ret,colon;  
	ret = url.find("http://");  
	if(ret != -1)  
	{  
		removeProtocol = url.substr(strlen("http://"));  
	}  
	else  
	{  
		removeProtocol = url;  
	}
	ret = removeProtocol.find_first_of('/');
	colon = removeProtocol.find_first_of(':');
	if(colon!=-1)
	{
		serverName = removeProtocol.substr(0,colon);
		port = removeProtocol.substr(colon+1,ret-colon-1);
	}
	else
	{
		serverName = removeProtocol.substr(0,ret);  
	}
	int net = removeProtocol.find_last_of('/');  
	sourcePath = removeProtocol.substr(ret,net-ret+1);  
	fileName = removeProtocol.substr(net+1);  
	//////////////////////////////////////////////////////////////////   
	WSADATA wsaData;  
	int nRet;  
	//   
	// Initialize WinSock.dll   
	//   
	nRet = WSAStartup(MAKEWORD(2,2), &wsaData);  
	if (nRet)  
	{
		PrintLog(L"WSAStartup error\r\n"); 
		WSACleanup();  
		return false;  
	}
	if ( 2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion) )
	{
		PrintLog(L"WSAStartup version error\r\n"); 
		WSACleanup();
		return false;
	}

	IN_ADDR        iaHost;  
	LPHOSTENT    lpHostEntry;   
	iaHost.s_addr = inet_addr(serverName.c_str());
	lpHostEntry = gethostbyname(serverName.c_str());  
	//if (iaHost.s_addr == INADDR_NONE)  
	//{  
	//	// Wasn't an IP address string, assume it is a name   
	//	lpHostEntry = gethostbyname(serverName.c_str());  
	//}  
	//else  
	//{  
	//	// It was a valid IP address string   
	//	lpHostEntry = gethostbyaddr((const char *)&iaHost,   
	//		sizeof(struct in_addr), AF_INET);  
	//	if(lpHostEntry == NULL)
	//		lpHostEntry = gethostbyname(serverName.c_str());  
	//}  
	if (lpHostEntry == NULL)  
	{  
		PrintLog(L"gethostbyname() error\r\n"); 
		WSACleanup();
		return false;  
	}  

	//       
	// Create a TCP/IP stream socket   
	//   
	SOCKET    parmSock;      

	parmSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
	if (parmSock == INVALID_SOCKET)  
	{
		PrintLog(L"socket() error %d\r\n",GetLastError());  
		WSACleanup();  
		return false;  
	}  


	//   
	// Find the port number for the HTTP service on TCP   
	//   
	LPSERVENT lpServEnt;  
	SOCKADDR_IN saServer;  

	lpServEnt = getservbyname("http", "tcp");  
	if (lpServEnt == NULL)  
		saServer.sin_port = htons(80);  
	else
	{
		if(port.compare("80")!=0)
		{
			saServer.sin_port = htons(atoi(port.c_str()));
		}
		else
		{
			saServer.sin_port = lpServEnt->s_port; 
		}
	}

	//   
	// Fill in the rest of the server address structure   
	//   
	saServer.sin_family = AF_INET;  
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);  

	//   
	// Connect the socket   
	//   
	nRet = connect(parmSock, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));  
	if (nRet == SOCKET_ERROR)  
	{  
		PrintLog(L"connect() error\r\n");  
		closesocket(parmSock);
		WSACleanup();  
		return false;  
	}  
	//////////////////////////////////////////////////////////////////   
	return client.DownloadFile(parmSock,serverName,sourcePath,fileName,port,buff,size);
}
//���ذ汾
//bool GetFileBuff(std::string url,OUT PBYTE *buff,OUT SIZE_T *size)
//{
//	char szDriverImagePath[MAX_PATH] = {0};
//
//	GetModuleFileNameA(NULL,szDriverImagePath,MAX_PATH); //�õ���ǰģ��·��
//	for (int i=strlen(szDriverImagePath);i>0;i--)
//	{
//		if ('\\'== szDriverImagePath[i])
//		{
//			szDriverImagePath[i+1]='\0';
//			break;
//		}
//	}
//	int index = url.find("WebRoot");
//
//	url = url.substr(index,url.length()-index);
//
//	string tmpstr = szDriverImagePath;
//	tmpstr +="..\\";
//	tmpstr += url;
//
//	FILE *fp;
//	PBYTE data=NULL;
//	size_t fsize;
//	fp = fopen(tmpstr.c_str(), "rb");
//	if (fp == NULL)
//	{
//		return false;
//	}
//
//	fseek(fp, 0, SEEK_END);
//	fsize = ftell(fp);
//	data = (PBYTE)malloc(fsize);
//	fseek(fp, 0, SEEK_SET);
//	fread(data, 1, fsize, fp);
//	*buff = data;
//	*size = fsize;
//	fclose(fp);
//	return true;
//}

bool DownloadFileA(const char* remoteFile, const char* localFile, bool isBreakContinue)
{
	if (NULL == remoteFile || NULL ==localFile)
		return false;

	InetManager inetM;
	if(!inetM.Init())
	{
		MessageBoxA(NULL,"�����ʼ��ʧ��","����",0);
		return false;
	}
	InetDownload* lpDownload = NULL;
	lpDownload = (InetDownload*)(inetM.CreateDownload());
	bool bSucess = true;

	if (lpDownload)
	{
		// ���س�tmp��β����ʱ���⣬����ȥ��tmp
		std::string strLocalFileTmp = localFile;
		strLocalFileTmp += ".tmp";

		lpDownload->SetFileUrl((inetM.CheckUrl(remoteFile)).c_str());
		if (isBreakContinue)
		{
			lpDownload->SetLocalFile(strLocalFileTmp.c_str());
			lpDownload->SetBreakAndContinue(isBreakContinue);
		}

		lpDownload->Start();
		while(true)//����ʱ�õ�д��
		{
			Sleep(100);
			if (0 == strcmp("finish", lpDownload->GetState()))
			{
				if (isBreakContinue)
				{
					bSucess = true;
					// ɾ���Ѿ��е��ļ���֮���ٸ�������Ҫ������
					remove(localFile);
					rename(strLocalFileTmp.c_str(), localFile);
				}
				else
				{
					if (!lpDownload->WriteToFile(localFile))
					{
						bSucess= false;
					}
				}
				break;
			}

			if (0 == strcmp("failed", lpDownload->GetState()))
			{
				bSucess = false;

				break;
			}

			if (0 == strcmp("opened", lpDownload->GetState()))
			{
				// ��߿����������ع����еģ���С���ȵķ���
			}

		}
		lpDownload->Stop();
		inetM.DeleteDownload((void*)lpDownload);
		inetM.Shut();
		return bSucess;

	}

	return false;
}

SIZE_T DownloadFileInBuff(const char* remoteFile, OUT PBYTE *buff, bool isBreakContinue)
{
	if (NULL == remoteFile || NULL ==buff)
		return false;

	InetManager inetM;
	if(!inetM.Init())
	{
		MessageBoxA(NULL,"�����ʼ��ʧ��","����",0);
		return false;
	}
	InetDownload* lpDownload = NULL;
	lpDownload = (InetDownload*)(inetM.CreateDownload());
	DWORD bSucess = 0;

	if (lpDownload)
	{

		lpDownload->SetFileUrl((inetM.CheckUrl(remoteFile)).c_str());
		if (isBreakContinue)
		{
			lpDownload->SetLocalFile("");
			lpDownload->SetBreakAndContinue(isBreakContinue);
		}

		lpDownload->Start();
		while(true)//����ʱ�õ�д��
		{
			Sleep(100);
			if (0 == strcmp("finish", lpDownload->GetState()))
			{
				if (isBreakContinue)
				{
					bSucess = lpDownload->GetFileSize();
				}
				else
				{
					if(lpDownload->GetFileSize())
					{
						*buff = (PBYTE)malloc(lpDownload->GetFileSize());
						memcpy(*buff,lpDownload->GetFileData(),lpDownload->GetFileSize());
						bSucess = lpDownload->GetFileSize();
					}
					else
					{
						bSucess = 0;
					}
				}
				break;
			}

			if (0 == strcmp("failed", lpDownload->GetState()))
			{
				bSucess = 0;

				break;
			}

			if (0 == strcmp("opened", lpDownload->GetState()))
			{
				// ��߿����������ع����еģ���С���ȵķ���
			}

		}
		lpDownload->Stop();
		inetM.DeleteDownload((void*)lpDownload);
		inetM.Shut();
		return bSucess;

	}

	return false;
}

bool DownloadFileB(const char* remoteFile, const char* localFile)
{
	PBYTE buff;
	SIZE_T size = 0;
	string url = remoteFile;
	if(!GetFileBuff(url,&buff,&size))
		return false;
	FILE *fp;
	fp = fopen(localFile, "wb");
	if (fp == NULL)
	{
		PrintLog(L"�޷������ر����ļ�[%S]\r\n",localFile);
		return false;
	}

	fwrite(buff, size, 1, fp);
	fclose(fp);	
	return true;
}
