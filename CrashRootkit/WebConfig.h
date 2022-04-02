#ifndef _WEBCONFIG_H_
#define _WEBCONFIG_H_

#include <string>
#include <vector>
#include <map>

using namespace std;

typedef map<string,string> CFGMap;
typedef vector<string> StringVector;

class CWebConfig
{
	CFGMap m_cfgmap;
public:
	CWebConfig();
	~CWebConfig();
	bool LoadURL(string url);
	bool LoadBuff(PBYTE buff, SIZE_T size,bool encrypt = true);
	bool WriteFile(const char * filepath,bool encrypt = true);
	string GetValue(string key);
	void SetValue(string key,string val);
	static StringVector GetStringVector(string val,const char* token);
	static StringVector * GetStringVector1(string val,const char* token);
	static char* decrypt(const char * buff, SIZE_T size);
	static char* encrypt(const char * buff, SIZE_T size);
};

void remove_space(string& str);

#endif