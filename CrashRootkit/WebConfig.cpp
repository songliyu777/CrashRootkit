#include "stdafx.h"
#include "WebConfig.h"
#include "HTTPClient.h"
#include "Log.h"
#ifdef RELEASE_VERSION
#include "VMProtectSDK.h"
#endif

#pragma warning(disable:4996)

CWebConfig::CWebConfig()
{

}

CWebConfig::~CWebConfig()
{
	
}

bool CWebConfig::LoadURL(string url)
{
	PBYTE buff;
	SIZE_T size;
	size = DownloadFileInBuff(url.c_str(),&buff,false);
	if (!size)
	{
		if(!GetFileBuff(url,&buff,&size))
			return false;
	}
	if(!LoadBuff(buff,size,false))
	{
		free(buff);
		return false;
	}
	free(buff);
	return true;
}

bool CWebConfig::LoadBuff(PBYTE buff, SIZE_T size,bool encrypt)
{
	string strbuff;
	if(encrypt)
	{
		char *debuff = decrypt((char *)buff,size);
		strbuff = (char *)debuff;
		free(debuff);
	}
	else
	{
		strbuff = (char *)buff;
	}
	int endoffset = strbuff.find("[END]");
	if(endoffset == -1)
	{
		PrintDbgString(L"无法找到结束标记 \r\n");
		return false;
	}
	int startoffset = strbuff.find("[START]");
	if(startoffset == -1)
	{
		PrintDbgString(L"无法找到开始标记 \r\n");
		return false;
	}
	startoffset += 0x7;
	strbuff = strbuff.substr(startoffset,endoffset - startoffset);
	char* outer = NULL,*inner = NULL;
	StringVector sv;
	for(outer = strtok( (char*)strbuff.c_str(), "\r\n") ; NULL != outer; outer = strtok(NULL, "\r\n"))
	{
		if(strlen(outer)>0)
		{
			sv.push_back(string(outer));
		}
	}
	StringVector::iterator pos;
	for(pos = sv.begin();pos!=sv.end();pos++)
	{
		bool iskey = true;
		string key,val;
		for(inner = strtok((char *)(*pos).c_str(), "="); NULL !=inner; inner = strtok(NULL, "="))
		{
			if(iskey)
			{
				key = inner;
				remove_space(key);
				iskey = false;
			}
			else
			{
				val = inner;
				remove_space(val);
			}
		}
		m_cfgmap.insert(CFGMap::value_type(key,val));
	}
	return true;
}

StringVector * CWebConfig::GetStringVector1(string val,const char* token)
{
	char* outer = NULL,*inner = NULL;
	StringVector * sv = new StringVector;
	for(outer = strtok((char*)val.c_str(), token) ; NULL != outer; outer = strtok(NULL, token))
	{
		if(strlen(outer)>0)
		{
			string tmp = string(outer);
			remove_space(tmp);
			sv->push_back(tmp);
		}
	}
	return sv;
}

StringVector CWebConfig::GetStringVector(string val,const char* token)
{
	char* outer = NULL,*inner = NULL;
	StringVector sv;
	for(outer = strtok((char*)val.c_str(), token) ; NULL != outer; outer = strtok(NULL, token))
	{
		if(strlen(outer)>0)
		{
			string tmp = string(outer);
			remove_space(tmp);
			sv.push_back(tmp);
		}
	}
	return sv;
}

char* CWebConfig::encrypt(const char * buff, SIZE_T size)
{
#ifdef RELEASE_VERSION
	VMProtectBegin("encrypt");
#endif
	char* enbuff = (char*)malloc(size);
	for(SIZE_T i = 0 ;i < size - 1;i++)
	{
		enbuff[i] = buff[i]^buff[i+1];
		if(i == size - 2)
		{
			enbuff[i+1] = buff[i+1];
		}
	}
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
	return enbuff;
}

char* CWebConfig::decrypt(const char * buff, SIZE_T size)
{
#ifdef RELEASE_VERSION
	VMProtectBegin("decrypt");
#endif
	char* enbuff = (char*)malloc(size);
	for(SIZE_T i = size - 1 ;i > 0;i--)
	{
		if(i == size - 1)
		{
			enbuff[i] = buff[i];
		}
		enbuff[i-1] = buff[i-1];
		enbuff[i-1] = enbuff[i-1]^enbuff[i];
	}
#ifdef RELEASE_VERSION
	VMProtectEnd();
#endif
	return enbuff;
}

bool CWebConfig::WriteFile(const char * filepath,bool bEncrypt)
{
	string buff;
	buff += "[START]\r\n";
	CFGMap::iterator pos;
	for(pos = m_cfgmap.begin();pos!=m_cfgmap.end();pos++)
	{
		buff += (*pos).first + " = " + (*pos).second + "\r\n";
	}
	buff += "[END]\r\n";
	FILE *fp;
	size_t size = 0;
	fp = fopen(filepath, "wb");
	if (fp == NULL)
	{
		::MessageBoxW(NULL,L"无法打开文件！",L"提示",0);
		return false;
	}
	if(bEncrypt)
	{
		char *enbuff = encrypt(buff.c_str(),buff.length());
		fwrite(enbuff, buff.length(), 1, fp);
		free(enbuff);
	}
	else
	{
		fwrite(buff.c_str(), buff.length(), 1, fp);
	}
	fclose(fp);	
	return true;
}

string CWebConfig::GetValue(string key)
{
	CFGMap::iterator pos;
	pos = m_cfgmap.find(key);
	if(pos != m_cfgmap.end())
	{
		return (*pos).second;
	}
	return string("");
}

void CWebConfig::SetValue(string key,string val)
{
	m_cfgmap[key] = val;
}

