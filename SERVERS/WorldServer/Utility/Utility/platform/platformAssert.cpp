//#include "stdafx.h"

#include "platformAssert.h"
#include "platformStrings.h"
#include <stdarg.h>
//#include "log.h"

PlatformAssert gAssert;
PlatformAssert* g_pAssert = NULL;

PlatformAssert::PlatformAssert()
{
	g_pAssert = this;
}

PlatformAssert::~PlatformAssert()
{	
}

void PlatformAssert::Output(Type logType, const char * filename, U32 line, const char* msg)
{	
	
}

void PlatformAssert::Crash(Type logType, const char* msg)
{
#ifdef DEBUG
	_asm int 3;
	Platform::forceShutdown(1);
#else
	char szMsg[2048] = {0};
	strcpy_s(szMsg, 2048, msg);
	if(logType <= PA_ERROR)
	{
		_asm int 3;
		//ExitProcess(1);
	}
#endif//DEBUG
}

void PlatformAssert::Assert(Type logType, const char * filename, U32 line, const char* msg, bool isChoice)
{
#ifdef DEBUG
	char content[2048];
	if(isChoice)
	{
		dSprintf(content, 2048, "文件： %s\n行号： %ld行\n描述： %s\n\n中断执行点＂确定＂，否则点＂取消＂",
			filename, line,msg);
		if(Platform::AlertOKCancel("代码异常", content))
		{
			_asm int 3;
		}
	}
	else
	{
		dSprintf(content, 2048, "代码文件： %s\n行号： %ld行\n描述： %s\n\n点＂确定＂可继续执行",
			filename, line,msg);
		Platform::AlertOK("代码异常", content);
	}

#endif//DEBUG
}

const char* avar(const char *message, ...)
{
   static char buffer[4096];
   va_list args;
   va_start(args, message);
   dVsprintf(buffer, sizeof(buffer), message, args);
   return( buffer );
}