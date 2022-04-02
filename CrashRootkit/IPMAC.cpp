#include "stdafx.h"
#include "IPMAC.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <httpext.h>
#include <windef.h>
#include <NtDDNdis.h>
#include <Nb30.h>
#include "ConsoleDll.h"
#include "winioctl.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")

string GetLocalIP()
{

	string strResult = "";

	WSADATA wsaData;

	char name[255];

	PHOSTENT hostinfo;  

	char *ip = NULL;	

	if ( WSAStartup( MAKEWORD(2,0), &wsaData ) == 0 )
	{   
		if( gethostname ( name, sizeof(name)) == 0)
		{  
			if((hostinfo = gethostbyname(name)) != NULL)
			{ 
				//获得IP
				ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);
				strResult = ip;
			}  

		}  
		WSACleanup();
	}
	return strResult;
}

string GetMac(string sIP)
{
	string strResult;
	char result[32] = {0};
	unsigned char mac[6]={0};
	ULONG  ulen = 6; 
	IPAddr destIP = inet_addr(sIP.c_str()); 
	if(NO_ERROR == SendARP(destIP,NULL,(PULONG)mac,&ulen))
	{
		sprintf_s(result,"%02X%02X%02X%02X%02X%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
		strResult = result;
	} 
	else
	{
		strResult = "";
	}
	if(!strResult.compare("000000000000"))
	{
		strResult = GetMacAddress();
	}
	return strResult;
}

string GetIPMAC()
{
	string strResult = "";
	char result[128] = {0};
	string ip = GetLocalIP();
	string mac;
	if(!ip.empty())
	{
		mac = GetMac(ip);
		if(!mac.empty())
		{
			sprintf_s(result,"[ip=%s][mac=%s]",ip.c_str(),mac.c_str());
			strResult = result;
		}
	}
	return strResult;
}

string GetPhyMacAddress(char* strServiceName)
{
	char  pstrBuf[512];
	sprintf_s(pstrBuf, "//./%s", strServiceName);
	string strResult = "";
	HANDLE  hDev  = CreateFileA(pstrBuf, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
	if ( hDev != INVALID_HANDLE_VALUE )
	{
		int    inBuf;
		BYTE  outBuf[256]  = { 0 };
		DWORD  BytesReturned;
		inBuf  = OID_802_3_PERMANENT_ADDRESS;

		if ( DeviceIoControl(hDev, IOCTL_NDIS_QUERY_GLOBAL_STATS, (LPVOID)&inBuf, 4, outBuf, 256, &BytesReturned, NULL) )
		{
			sprintf_s(pstrBuf, "%02X%02X%02X%02X%02X%02X",
				outBuf[0], outBuf[1], outBuf[2], outBuf[3], outBuf[4], outBuf[5]);
			strResult = pstrBuf;
		}

		inBuf  = OID_802_3_CURRENT_ADDRESS;
		if ( DeviceIoControl(hDev, IOCTL_NDIS_QUERY_GLOBAL_STATS, (LPVOID)&inBuf, 4, outBuf, 256, &BytesReturned, NULL) )
		{
			sprintf_s(pstrBuf, "%02X%02X%02X%02X%02X%02X",
				outBuf[0], outBuf[1], outBuf[2], outBuf[3], outBuf[4], outBuf[5]);
			strResult = pstrBuf;
		}
		CloseHandle(hDev);
	}
	return strResult;
}

//  网卡 MAC 地址
string GetMacAddress()
{
	UINT      uErrorCode  = 0;
	IP_ADAPTER_INFO  iai;
	ULONG      uSize    = 0;
	DWORD      dwResult  = GetAdaptersInfo(&iai, &uSize);
	string mac = "";
	if ( dwResult == ERROR_BUFFER_OVERFLOW )
	{
		PIP_ADAPTER_INFO  piai  = (PIP_ADAPTER_INFO)new BYTE[uSize];
		PIP_ADAPTER_INFO  piai_tmp = piai;
		dwResult  = GetAdaptersInfo(piai, &uSize);
		if ( ERROR_SUCCESS == dwResult )
		{
			while ( piai_tmp )
			{
				//PrintDbgString(L"名称:%S\n", piai->AdapterName);
				//PrintDbgString(L"描述:%S\n", piai->Description);
				//PrintDbgString(L"类型:%d\n", piai->Type);
				mac = GetPhyMacAddress(piai_tmp->AdapterName);
				if(!mac.empty())
				{
					break;
				}
				piai_tmp = piai_tmp->Next;
			}
		}
		delete[] piai;
	}
	return mac;
}
