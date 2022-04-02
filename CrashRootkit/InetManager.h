//--------------------------------------------------------------------
// �ļ���:		InetManager.h
// ��  ��:		
// ˵  ��:		
// ��������:	2009��6��13��
// ������:		½����
// ��Ȩ����:	������ţ�������޹�˾
//--------------------------------------------------------------------

#ifndef _INETMANAGER_H
#define _INETMANAGER_H

#include <string>
#include <windows.h>
#include <wininet.h>

// �������ӹ���

class InetManager
{
public:
	InetManager();
	virtual ~InetManager();

	virtual bool Init();
	virtual bool Shut();
	
	// �����ļ����ض���
	void* CreateDownload();
	// ɾ���ļ����ض���
	bool DeleteDownload(void *pVoid);

	// ���URL��������ȷ��·������
	std::string CheckUrl(const char* url);

private:
	HINTERNET m_hSession;
};

#endif // _INETMANAGER_H
