//-------------------------------------------------------------------- 
// 文件名	:	Macros.h
// 内  容	:	公用的宏定义
// 说  明	:
// 创建日期	:	2002年7月30日
// 最后修改	:	2002年7月30日
// 创建人	:	陆利民
// 版权所有	:	苏州蜗牛电子有限公司
//--------------------------------------------------------------------

#ifndef _MACROS_H
#define _MACROS_H

#ifdef _WIN32
#define FX_SYSTEM_WINDOWS
#ifdef _WIN64
#define FX_SYSTEM_64BIT
#else
#define FX_SYSTEM_32BIT
#endif // _WIN64
#endif // _WIN32

#ifdef __linux__
#define FX_SYSTEM_LINUX
#define FX_SYSTEM_64BIT
#endif // __linux__

#ifdef FX_SYSTEM_WINDOWS
#include <stddef.h>
#pragma warning(disable: 4786)
#pragma warning(disable: 4996)
#pragma warning(disable: 4819)
#define FX_DLL_EXPORT __declspec(dllexport)
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif // FX_SYSTEM_WINDOWS

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)	{ if (p) { (p)->Release(); (p) = NULL; } }
#endif // !SAFE_RELEASE

#pragma warning(disable:4786)

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

#define WORD1(p)		(p & 0xFFFF)
#define WORD2(p)		((p >> 16) & 0xFFFF)
#define BYTE1(p)		(p & 0xFF)
#define BYTE2(p)		((p >> 8) & 0xFF)
#define BYTE3(p)		((p >> 16) & 0xFF)
#define BYTE4(p)		((p >> 24) & 0xFF)

typedef unsigned char	UI08;
typedef unsigned short	UI16;
typedef unsigned int	UI32;
typedef char			SI08;
typedef short			SI16;
typedef int				SI32;

typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;

#define	UI32_MAX		0xFFFFFFFF
#define UI16_MAX		0xFFFF
#define UI08_MAX		0xFF

// SOCKET句柄
typedef size_t			SOCK_HANDLE;

#include "Debug.h"

void __cdecl _TraceInfo(const char * info, ...);
void __cdecl _EchoInfo(const char * info, ...);

#ifndef NDEBUG
	#define LogMessage(p)	_EchoInfo("Message, %s, %d, %s", __FILE__, __LINE__, p); _TraceInfo("Message, %s, %d, %s", __FILE__, __LINE__, p)
	#define LogError(p)		_EchoInfo("Error, %s, %d, %s", __FILE__, __LINE__, p); _TraceInfo("Error, %s, %d, %s", __FILE__, __LINE__, p)
	#define LogWarning(p)	_EchoInfo("Warning, %s, %d, %s", __FILE__, __LINE__, p); _TraceInfo("Warning, %s, %d, %s", __FILE__, __LINE__, p)
#else
	#define LogMessage(p)	_TraceInfo("Message, %s", p)
	#define LogError(p)		_TraceInfo("Error, %s", p)
	#define LogWarning(p)	_TraceInfo("Warning, %s", p)
#endif

#endif // _MACROS_H

