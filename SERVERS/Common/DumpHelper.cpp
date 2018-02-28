#undef _STRSAFE_H_INCLUDED_
#include "DumpHelper.h"
#include <stdlib.h>
#include <shellapi.h>
#include <io.h>
#include <tchar.h>
#include <strsafe.h>

#include "PacketType.h"
#include <process.h>
#include "Base/Log.h"

#include "Commlib/VirtualConsole.h"
#ifdef NTJ_SERVER
#include "Gameplay/ServerGameplayState.h"
#endif

//#pragma comment(lib, "dbghelp.lib")

CDumpHelper g_dumper;

LPTOP_LEVEL_EXCEPTION_FILTER CDumpHelper::DefaultFail = NULL;

int catchError(unsigned int code, _EXCEPTION_POINTERS* EP,int dumpType)
{
	g_Log.WriteError(DumpHelper::DumpStack());
	g_dumper.writeDump(GetCurrentThreadId(), EP, dumpType);
	return EXCEPTION_EXECUTE_HANDLER;
}

void debugDump(int dumpType)
{
#ifndef NO_DUMP

	__try
	{
		int *p=NULL;
		*p=1;
	}
	__except( catchError(GetExceptionCode(),GetExceptionInformation(),dumpType) )
	{
	}

#else

	int *p=NULL;
	*p=1;
#endif
}

#ifdef WORLDSERVER

#endif /*WORLDSERVER*/

LONG WINAPI CDumpHelper::SystemFail( LPEXCEPTION_POINTERS pExceptionPointers )
{
#ifdef SHOW_DUMP_DIALOG
	MessageBoxA( 0, "由于一个未知的异常导正应用程序异常终止,点击确定产生DUMP文件", "错误", 0 );
#endif

	catchDump(pExceptionPointers);
	//BOOL bMiniDumpSuccessful; 
	//bMiniDumpSuccessful = writeDump( pExceptionPointers );

    if (IsDebuggerPresent())
        return EXCEPTION_CONTINUE_SEARCH;
    else
        return EXCEPTION_EXECUTE_HANDLER;
}

void CDumpHelper::catchDump(LPEXCEPTION_POINTERS pExceptionPointers)
{
#ifndef NTJ_ROBOT
	//Ray: 起个线程来写，防止堆栈溢出
	CLocker lock(g_dumper.m_CS);
	//printf("\n捕获线程号 %d\n",GetCurrentThreadId());
	g_dumper.m_Ptr = pExceptionPointers;
	g_dumper.m_dumpThreadID = GetCurrentThreadId();
	SetEvent(g_dumper.m_hBegin);
	WaitForSingleObject(g_dumper.m_hEnd, INFINITE);
#else
    g_dumper.writeDump(GetCurrentThreadId(), pExceptionPointers);
#endif
}

void CDumpHelper::dumpCallback(void *pThis)
{
	HANDLE list[2] = {g_dumper.m_hBegin,g_dumper.m_hQuit};

	while(1)
	{
		DWORD retVal =WaitForMultipleObjects(2,list,false,INFINITE);
		if (WAIT_OBJECT_0+1 == retVal)
		{
			return;
		}
		else
		{
			//printf("\nwriteDump线程号 %d\n",GetCurrentThreadId());

			BOOL bMiniDumpSuccessful; 
			bMiniDumpSuccessful = g_dumper.writeDump(g_dumper.m_dumpThreadID, g_dumper.m_Ptr);
			SetEvent(g_dumper.m_hEnd);
		}
	}
}

CDumpHelper::CDumpHelper()
:m_dumpThreadID(0)
{
#ifndef NTJ_SERVER
	char moduleName[MAX_PATH];
	char buf[128];

	GetModuleFileNameA( NULL, moduleName, MAX_PATH );
	const char* endChar = strrchr( moduleName, '\\' );

	char name[128];
	strncpy_s( name, sizeof( name ), moduleName, endChar - moduleName );

	const char* rendChar = strrchr( name, '\\' );
#endif

	m_hBegin = CreateEvent(NULL,false,false,NULL);
	m_hEnd = CreateEvent(NULL,false,false,NULL);
	m_hQuit = CreateEvent(NULL,true,false,NULL);

#ifndef NTJ_ROBOT
	m_hThread = (HANDLE)_beginthread(dumpCallback,0,this);
#else
    m_hThread = 0;
#endif

	SetHandler();
}

CDumpHelper::~CDumpHelper()
{
	SetEvent(m_hQuit);

    if (0 != m_hThread)
    {
	    WaitForSingleObject(m_hThread,INFINITE);
    }

	CloseHandle(m_hBegin);
	CloseHandle(m_hEnd);
	CloseHandle(m_hQuit);
}

void CDumpHelper::reset()
{
	LPTOP_LEVEL_EXCEPTION_FILTER fail = ::SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER)&SystemFail );
	
	if( fail != (LPTOP_LEVEL_EXCEPTION_FILTER)&SystemFail )
	{
		DefaultFail = fail;
	}
}

BOOL CDumpHelper::writeDump(DWORD threadID,	LPEXCEPTION_POINTERS pExceptionPointers, int dumpType)
{
	BOOL bMiniDumpSuccessful;
	
	char szPath[MAX_PATH]; 
	char szFileName[MAX_PATH]; 
	char szAppName[MAX_PATH] = { 0 }; // = TEXT("Dump"); 
	DWORD dwBufferSize = MAX_PATH; 
	HANDLE hDumpFile;  
	SYSTEMTIME stLocalTime; 
	MINIDUMP_EXCEPTION_INFORMATION ExpParam; 

	GetLocalTime( &stLocalTime ); 
	GetCurrentDirectoryA( dwBufferSize, szPath ); 

	CreateDirectoryA( "Dumps", NULL ); 

	::GetModuleFileNameA( NULL, szAppName,MAX_PATH );
	char* pChar = strrchr( szAppName, '\\' );
	pChar++;

	char* sChar = strrchr( pChar, '.' );
	*sChar = 0;
#ifdef NTJ_SERVER
	StringCchPrintfA( szFileName, MAX_PATH, "Dumps\\%s[%d][%s]-[%d]-%04d%02d%02d-%02d%02d%02d.dmp", 
		pChar, g_nZoneId, G_PVERSION, GetCurrentProcessId(),
		stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond 
		); 
#else
	StringCchPrintfA( szFileName, MAX_PATH, "Dumps\\%s[%s]-[%d]-%04d%02d%02d-%02d%02d%02d.dmp", 
		pChar, G_PVERSION, GetCurrentProcessId(),
		stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond 
		); 
#endif
	hDumpFile = CreateFileA(szFileName, GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0); 

	ExpParam.ThreadId = threadID; 
	ExpParam.ExceptionPointers = pExceptionPointers; 
	ExpParam.ClientPointers = TRUE; 

	bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
		hDumpFile, static_cast<MINIDUMP_TYPE>(dumpType), &ExpParam, NULL, NULL); 

	CloseHandle(hDumpFile);	


#ifdef NTJ_CLIENT
	//显示CrashReport
	ShellExecuteA(0,"open","Bin/CrashReport.exe",szFileName,0,SW_SHOW);
#endif

	return bMiniDumpSuccessful;
}

void CDumpHelper::SetHandler()
{
#ifndef NO_DUMP
	if (!IsDebuggerPresent())
		DefaultFail = ::SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER)&SystemFail );
#endif
}

CVersion& CVersion::GetInstance()
{
	static CVersion s_version;
	return s_version;
}

#ifdef NTJ_CLIENT
	#define GAME_VERSION_FILE "config/client.ini"
#else
	#define GAME_VERSION_FILE "./server.ini"
#endif

CVersion::CVersion()
#ifndef NTJ_CLIENT
:m_loadInterval(0), m_isFileExist(false)
#endif
{
#ifndef NTJ_CLIENT
    m_sVer = 0;
	int bv1, bv2, bv3, bv4 = 0;
	sscanf_s(G_PVERSION, "%d,%d,%d,%d", &bv1, &bv2, &bv3, &bv4);
	m_sBuildVer = bv1 * 1000*1000*1000 + bv2 * 1000*1000 + bv3 * 1000 + bv4;
#endif

	LoadIni();
}

void CVersion::LoadIni()
{
#ifndef NTJ_CLIENT
	if (_access(GAME_VERSION_FILE, 0) != -1)
	{
		m_isFileExist = true;
#endif 
		GetPrivateProfileStringA("Version",	"GameVersion", "", m_vesion, COMMON_STRING_LENGTH, GAME_VERSION_FILE);
		GetPrivateProfileStringA("Version",	"MD5", "", m_md5, COMMON_STRING_LENGTH, GAME_VERSION_FILE);
#ifndef NTJ_CLIENT

        //读取版本号的每个块
        if (0 == strlen(m_vesion))
        {
            m_isFileExist = false;
        }
        else
        {
            int v1, v2, v3, v4 = 0;
	        sscanf_s(m_vesion, "%d.%d.%d.%d", &v1, &v2, &v3, &v4);
            m_sVer = v1 * 1000*1000*1000 + v2 * 1000*1000 + v3 * 1000 + v4;
        }
	}   
	else
	{
		m_isFileExist = false;
	}
#endif 
}

#ifndef NTJ_CLIENT
void CVersion::TimeProcess(U32 bExit)
{
	m_loadInterval++;
	if (m_loadInterval > 10)
	{
		m_loadInterval = 0;
		LoadIni();
	}
	//return true;
}

bool CVersion::IsAcceptableVersion(char* version)
{
	if (m_isFileExist && 0 != version)
    {
        int v[4] = {0};
	    sscanf_s(version, "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]);
        __int64 sClinet = v[0] * 1000*1000*1000 + v[1] * 1000*1000 + v[2] * 1000 + v[3];

		return sClinet >= m_sVer;
    }

	return true;
}

bool CVersion::IsAcceptableBuildVersion(char* version)
{
	int v[4] = {0};
	sscanf_s(version, "%d,%d,%d,%d", &v[0], &v[1], &v[2], &v[3]);
	__int64 sClinet = v[0] * 1000*1000*1000 + v[1] * 1000*1000 + v[2] * 1000 + v[3];

	return sClinet >= m_sBuildVer;
}
#endif