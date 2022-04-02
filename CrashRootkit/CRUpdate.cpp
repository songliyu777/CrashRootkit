#include "StdAfx.h"
#include "WebConfig.h"
#include "CRUpdate.h"
#include "Log.h"
#include "md5.h"
#include "HTTPClient.h"
#include "FlashDialog.h"
#include "IniConsole.h"

#pragma warning(disable:4996)

bool InitSysFile(OUT char * buf,IN const char * filename)
{
	if(!buf)
		return false;
	char szDriverImagePath[MAX_PATH] = {0};

	GetModuleFileNameA(NULL,szDriverImagePath,MAX_PATH); //得到当前模块路径
	for (int i=strlen(szDriverImagePath);i>0;i--)
	{
		if ('\\'== szDriverImagePath[i])
		{
			szDriverImagePath[i+1]='\0';
			break;
		}
	}
	strcat(szDriverImagePath,filename);
	strcpy(buf,szDriverImagePath);
	return true;
}

bool FileMD5Vaild(string filename,string filemd5)
{
	char *md5;
	//获取执行文件
	FILE *fp;
	unsigned char *data=NULL;
	size_t fsize;
	char filepath[MAX_PATH];
	InitSysFile(filepath,filename.c_str());//获取路径
	fp = fopen(filepath, "rb");
	if (fp == NULL)
	{
		PrintDbgString(L"无法打开[%S]文件 \r\n",filepath);
		PrintLog(L"无法打开[%S]文件 \r\n",filepath);
		return false;
	}
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	data = (unsigned char *)malloc(fsize);
	fseek(fp, 0, SEEK_SET);
	fread(data, 1, fsize, fp);
	md5 = MD5Buffer((PBYTE)data,fsize);
	if(strcmp(filemd5.c_str(),md5)!=0)//MD5验证
	{
		PrintDbgString(L"文件[%S] md5验证失败 md5(1):%S md5(2):%S \r\n",filepath,filemd5.c_str(),md5);
		PrintLog(L"文件[%S] md5验证失败 md5(1):%S md5(2):%S \r\n",filepath,filemd5.c_str(),md5);
		free(md5);
		fclose(fp);
		free(data);
		return false;
	}
	free(md5);
	fclose(fp);
	free(data);
	return true;
}

CWebConfig* LoadWebConfig(const char* url)
{
	CWebConfig *pWebcfg = new CWebConfig();
	string webconfigurl = url;
	webconfigurl += WEBCONFIG;
	if(pWebcfg->LoadURL(webconfigurl))
	{
		PrintDbgString(L"获取网络配置文件成功\r\n");
		PrintLog(L"获取网络配置文件成功\r\n");
		return pWebcfg;
	}
	delete pWebcfg;
	return NULL;
}

bool DownloadFile(const char* url,string filename)
{
	char filepath[MAX_PATH];
	InitSysFile(filepath,filename.c_str());//获取路径
	string webfilepath = url;
	webfilepath += filename;
	bool success = DownloadFileA(webfilepath.c_str(),filepath,true);
	if(!success)
	{
		success = DownloadFileB(webfilepath.c_str(),filepath);
		if(!success)
		{
			PrintDbgString(L"下载[%S]失败 \r\n",filename.c_str());
			PrintLog(L"下载[%S]失败 \r\n",filename.c_str());
		}
	}
	else
	{
		PrintDbgString(L"下载[%S]成功 \r\n",filename.c_str());
		PrintLog(L"下载[%S]成功 \r\n",filename.c_str());
	}
	return success;
}

bool RecoverLogo(CWebConfig * pWebcfg,const char* url)
{
	PVOID tmpbuff = NULL;
	string logo = pWebcfg->GetValue(LOGO);
	string logomd5 = pWebcfg->GetValue(LOGO_MD5);
	while(!FileMD5Vaild(logo,logomd5))
	{
		if(!DownloadFile(url,logo))
		{
			PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,logo.c_str());
			PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,logo.c_str());
			return false;
		}
	}
	return true;
}

bool RecoverFile(CWebConfig * pWebcfg,const char* url)
{
	string cca = pWebcfg->GetValue(CCA);
	string cca_md5 = pWebcfg->GetValue(CCA_MD5);
	if(!FileMD5Vaild(cca,cca_md5))
	{
		string strLocalFileTmp = cca;
		strLocalFileTmp += ".tmp";
		char filepath1[MAX_PATH],filepath2[MAX_PATH];
		InitSysFile(filepath1,strLocalFileTmp.c_str());//获取路径
		InitSysFile(filepath2,cca.c_str());//获取路径
		remove(filepath1);
		DeleteFileA(filepath1);
		remove(filepath2);
		DeleteFileA(filepath2);
	}
	while(!FileMD5Vaild(cca,cca_md5))
	{
		string strLocalFileTmp = cca;
		strLocalFileTmp += ".tmp";
		char filepath1[MAX_PATH],filepath2[MAX_PATH];
		InitSysFile(filepath1,strLocalFileTmp.c_str());//获取路径
		InitSysFile(filepath2,cca.c_str());//获取路径
		remove(filepath1);
		DeleteFileA(filepath1);
		rename(filepath2, filepath1);
		if(!DownloadFile(url,cca))
		{
			PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,cca.c_str());
			PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,cca.c_str());
			return false;
		}
	}
	UpdateValue(25);
	string yyf2 = pWebcfg->GetValue(YYF2);
	string yyf2_md5 = pWebcfg->GetValue(YYF2_MD5);
	if(!FileMD5Vaild(yyf2,yyf2_md5))
	{
		string strLocalFileTmp = yyf2;
		strLocalFileTmp += ".tmp";
		char filepath1[MAX_PATH],filepath2[MAX_PATH];
		InitSysFile(filepath1,strLocalFileTmp.c_str());//获取路径
		InitSysFile(filepath2,yyf2.c_str());//获取路径
		remove(filepath1);
		DeleteFileA(filepath1);
		remove(filepath2);
		DeleteFileA(filepath2);
	}
	while(!FileMD5Vaild(yyf2,yyf2_md5))
	{
		string strLocalFileTmp = yyf2;
		strLocalFileTmp += ".tmp";
		char filepath1[MAX_PATH],filepath2[MAX_PATH];
		InitSysFile(filepath1,strLocalFileTmp.c_str());//获取路径
		InitSysFile(filepath2,yyf2.c_str());//获取路径
		remove(filepath1);
		DeleteFileA(filepath1);
		rename(filepath2, filepath1);
		if(!DownloadFile(url,yyf2))
		{
			PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,yyf2.c_str());
			PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,yyf2.c_str());
			return false;
		}
	}
	UpdateValue(40);
	string moduleinject = pWebcfg->GetValue(MODULEINJECT);
	string moduleinject_md5 = pWebcfg->GetValue(MODULEINJECT_MD5);
	while(!FileMD5Vaild(moduleinject,moduleinject_md5))
	{
		if(!DownloadFile(url,moduleinject))
		{
			PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,moduleinject.c_str());
			PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,moduleinject.c_str());
			return false;
		}
	}
	UpdateValue(50);
	string yyfpatch = pWebcfg->GetValue(YYFPATCH);
	string yyfpatch_md5 = pWebcfg->GetValue(YYFPATCH_MD5);
	while(!FileMD5Vaild(yyfpatch,yyfpatch_md5))
	{
		if(!DownloadFile(url,yyfpatch))
		{
			PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,yyfpatch.c_str());
			PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,yyfpatch.c_str());
			return false;
		}
	}
	CIniConsole console;
	GlobleInfo gi = console.GetGlobleIni();
	UpdateValue(75);
	if(gi.BIGMAP_UPDATE)
	{
		string bigmap = pWebcfg->GetValue(BIGMAP);
		string bigmap_md5 = pWebcfg->GetValue(BIGMAP_MD5);
		while(!FileMD5Vaild(bigmap,bigmap_md5))
		{
			if(!DownloadFile(url,bigmap))
			{
				PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,bigmap.c_str());
				PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,bigmap.c_str());
				return false;
			}
		}
	}
	UpdateValue(80);
	if(gi.ITEM_UPDATE)
	{
		string item = pWebcfg->GetValue(ITEM);
		string item_md5 = pWebcfg->GetValue(ITEM_MD5);
		while(!FileMD5Vaild(item,item_md5))
		{
			if(!DownloadFile(url,item))
			{
				PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,item.c_str());
				PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,item.c_str());
				return false;
			}
		}
	}
	UpdateValue(80);
	if(gi.MAP_UPDATE)
	{
		string map = pWebcfg->GetValue(MAP);
		string map_md5 = pWebcfg->GetValue(MAP_MD5);
		while(!FileMD5Vaild(map,map_md5))
		{
			if(!DownloadFile(url,map))
			{
				PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,map.c_str());
				PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,map.c_str());
				return false;
			}
		}
	}
	UpdateValue(85);
	if(gi.SKILL_UPDATE)
	{
		string skill = pWebcfg->GetValue(SKILL);
		string skill_md5 = pWebcfg->GetValue(SKILL_MD5);
		while(!FileMD5Vaild(skill,skill_md5))
		{
			if(!DownloadFile(url,skill))
			{
				PrintDbgString(L"无法获取URL[%S]文件[%S] \r\n",url,skill.c_str());
				PrintLog(L"无法获取URL[%S]文件[%S] \r\n",url,skill.c_str());
				return false;
			}
		}
	}
	UpdateValue(90);
	return false;
}