#include "stdafx.h"
#include "ConsoleDll.h"
#include "CRString.h"

void PBYTE2STRING(PBYTE buf,SIZE_T count,string & str)
{
	str="";
	char num[3];
	string numstr,xstr = "x";
	for(SIZE_T i=0;i<count;i++)
	{
		BYTE b = buf[i];
		_itoa_s((int)b & 0xff,num,16);
		numstr = num;
		str+=(xstr+numstr);
	}
}

void EncryptString(string & src)
{
	char buf[4096] = {0};
	strcpy_s(buf,src.c_str());
	int len = strlen(buf);
	for(int i=0;i<len-1;i++)
	{
		buf[i]=buf[i]^buf[i+1];
	}
	PBYTE2STRING((PBYTE)buf,len,src);
}

void remove_space(string & str)
{ 
	string buff(str); 
	char space = ' '; 
	str.assign(buff.begin() + buff.find_first_not_of(space), 
		buff.begin() + buff.find_last_not_of(space) + 1); 
}

StringVector GetStringVector(string val,const char* token)
{
	char* outer = NULL,*inner = NULL,*result = NULL;;
	StringVector sv;
	for(outer = strtok_s((char*)val.c_str(), token, &result) ; NULL != outer; outer = strtok_s(NULL, token, &result))
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

void DecryptString(string & src)
{
	char buf[1024] = {0};
	string val = src;
	StringVector sv = GetStringVector(val,"x");
	size_t size = sv.size();
	if(size==0)
	{
		return;
	}
	char *endp;
	for(size_t i=0;i<size;i++)
	{
		string num = sv[i];
		long l = strtol(num.c_str(),&endp,16);
		buf[i] = (BYTE)(l & 0xff);
	}
	for(int i = size - 1;i > 0;i--)
	{
		buf[i-1]=(char) (buf[i-1]^buf[i]);
	}
	src = buf;
}