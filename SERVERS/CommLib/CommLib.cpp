#include "stdafx.h"
#include "CommLib.h"
#include "base/log.h"
#include <windows.h>
#include <process.h>
#include <WinIoCtl.h>
#include <psapi.h>

U32 getZeroTime(U32 t)
{
	struct tm today;
	_localtime32_s(&today, (__time32_t*)&t);
	today.tm_hour = 0;
	today.tm_min = 0;
	today.tm_sec = 0;
	return _mktime32(&today);
}

TickHelper::TickHelper(void)
{
    m_startTick = GetTickCount64();
}

TickHelper::~TickHelper(void)
{
    g_Log.WriteLog("tick count:%I64d", GetTickCount64() - m_startTick);
}

EventGuard::EventGuard(const char* name)
{
    assert(0 != name);

    m_pEventHandle = (void*)CreateEvent(0,0,TRUE,name);
}

EventGuard::~EventGuard(void)
{
    if (0 != m_pEventHandle)
    {
        CloseHandle((HANDLE)m_pEventHandle);
    }
}

bool EventGuard::CheckExist(const char* name)
{
    HANDLE et = OpenEvent(EVENT_ALL_ACCESS,0,name);

    if (0 == et)
        return false;

    CloseHandle(et);
    return true;
}

#pragma comment(lib, "psapi.lib")

U64 GetUsePhysMemory(void)
{
    PROCESS_MEMORY_COUNTERS process_memstate;
    memset(&process_memstate,0,sizeof(PROCESS_MEMORY_COUNTERS));
    GetProcessMemoryInfo(GetCurrentProcess(),&process_memstate, sizeof(PROCESS_MEMORY_COUNTERS));

    return process_memstate.WorkingSetSize /1024;
}

void DumpUsedMemory(void)
{
    PROCESS_MEMORY_COUNTERS process_memstate;
    memset(&process_memstate,0,sizeof(PROCESS_MEMORY_COUNTERS));
    GetProcessMemoryInfo(GetCurrentProcess(),&process_memstate, sizeof(PROCESS_MEMORY_COUNTERS));

    U64 workingSize     = process_memstate.WorkingSetSize /1024;
    U64 workingPeekSize = process_memstate.PeakWorkingSetSize /1024;
    U64 pageSize        = process_memstate.PagefileUsage /1024;
    U64 pagePeekSize    = process_memstate.PeakPagefileUsage /1024;

    g_Log.WriteLog("memory used[%I64d]/[%I64d]-[%I64d]/[%I64d]",workingSize,workingPeekSize,pageSize,pagePeekSize);
}

void SetConsole(std::string title, std::string version, std::string Ip, std::string Port)
{
	char buf[512];
	char ModuleStartTime[MAX_PATH] = { 0 };
	GetCurrentDirectoryA(256, ModuleStartTime);
	//SYSTEMTIME st;
	//GetLocalTime(&st);
	//sprintf_s(ModuleStartTime, MAX_PATH, "StartTime:[%02d/%02d/%04d]-[%02d:%02d:%02d]", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);

	sprintf_s(buf, sizeof(buf), "[MD]--S: %s, Ip :[%s:%s], V: [%s]-%s", title.c_str(), Ip.c_str(), Port.c_str(), version.c_str(), ModuleStartTime);
	SetConsoleTitleA(buf);
}