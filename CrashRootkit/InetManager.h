//--------------------------------------------------------------------
// 文件名:		InetManager.h
// 内  容:		
// 说  明:		
// 创建日期:	2009年6月13日
// 创建人:		陆利民
// 版权所有:	苏州蜗牛电子有限公司
//--------------------------------------------------------------------

#ifndef _INETMANAGER_H
#define _INETMANAGER_H

#include <string>
#include <windows.h>
#include <wininet.h>

// 外网连接管理

class InetManager
{
public:
	InetManager();
	virtual ~InetManager();

	virtual bool Init();
	virtual bool Shut();
	
	// 创建文件下载对象
	void* CreateDownload();
	// 删除文件下载对象
	bool DeleteDownload(void *pVoid);

	// 检查URL消除不正确的路径符号
	std::string CheckUrl(const char* url);

private:
	HINTERNET m_hSession;
};

#endif // _INETMANAGER_H
