#include "stdafx.h"

#include "Thread.h"
#include <windows.h>

static DWORD WINAPI ThreadProc(void* pvArg);

CThread::CThread(ThreadProcedure* pkProcedure):
    m_eStatus(SUSPENDED),
    m_uiReturnValue(0xFFFFFFFF),
    m_hThread(0)
{
    assert(pkProcedure);
    m_pkProcedure = pkProcedure;
}

CThread::~CThread(void)
{
    WaitForCompletion();
    m_pkProcedure = 0;
    
    if (m_hThread)
        CloseHandle((HANDLE)m_hThread);
        
    m_hThread = 0;
}

CThread* CThread::Create(ThreadProcedure* pkProcedure)
{
    CThread* pThread = new CThread(pkProcedure);
    
    if (pThread)
    {
        if (!pThread->SystemCreateThread())
        {
            delete pThread;
            pThread = 0;
        }
    }
    return pThread;
}

bool CThread::SetPriority(CThread::Priority ePriority)
{
    return SystemSetPriority(ePriority);
}

int CThread::Suspend()
{
    return SystemSuspend();
}

int CThread::Resume()
{
    return SystemResume();
}

bool CThread::WaitForCompletion()
{
    return SystemWaitForCompletion();
}

DWORD WINAPI ThreadProc(void* pvArg)
{
    CThread* pThread = (CThread*)pvArg;
    pThread->m_uiReturnValue = pThread->m_pkProcedure->run();
    unsigned int uiReturn = pThread->m_uiReturnValue;
    pThread->m_eStatus = CThread::COMPLETE;
    return uiReturn;
}

bool CThread::SystemCreateThread()
{
    if (!m_pkProcedure)
        return false;

    m_hThread = (void*)CreateThread(0,0,ThreadProc, this, CREATE_SUSPENDED, NULL);
    
    if (m_hThread == 0)
        return false;

    m_ePriority = NORMAL;
    m_eStatus = SUSPENDED;

    return true;
}

bool CThread::SystemSetPriority(Priority ePriority)
{
    if (m_ePriority != ePriority)
    {
        int iPriority;

        switch (ePriority)
        {
        case ABOVE_NORMAL:
            iPriority = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        case BELOW_NORMAL:
            iPriority = THREAD_PRIORITY_BELOW_NORMAL;
            break;
        case HIGHEST:
            iPriority = THREAD_PRIORITY_HIGHEST;
            break;
        case IDLE:
            iPriority = THREAD_PRIORITY_IDLE;
            break;
        case LOWEST:
            iPriority = THREAD_PRIORITY_LOWEST;
            break;
        case NORMAL:
            iPriority = THREAD_PRIORITY_NORMAL;
            break;
        case TIME_CRITICAL:
            iPriority = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        default:
            return false;
        }
        
        if (!SetThreadPriority((HANDLE)m_hThread, iPriority))
            return false;

        m_ePriority = ePriority;
    }

    return true;
}

int CThread::SystemSuspend()
{
    if (m_hThread == 0)
        return -1;

    int iRet = SuspendThread((HANDLE)m_hThread);
    
    if (iRet != -1)
        m_eStatus = SUSPENDED;
        
    return iRet;
}

int CThread::SystemResume()
{
    if (m_hThread == 0)
        return -1;

    int iPreviousSuspendCount = ResumeThread((HANDLE)m_hThread);
    switch (iPreviousSuspendCount)
    {
    case -1: 
        break;
    case 0: // fall through
    case 1:
        m_eStatus = RUNNING;
        break;
    default:
        m_eStatus = SUSPENDED;
    }
    return iPreviousSuspendCount;
}

bool CThread::SystemWaitForCompletion()
{
    if (m_eStatus == RUNNING)
    {
        WaitForSingleObject((HANDLE)m_hThread, INFINITE);
        m_eStatus = COMPLETE;
        return true;
    }
    else
    {
        return false;
    }
}