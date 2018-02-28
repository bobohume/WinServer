#ifndef _THREAD_H_
#define _THREAD_H_

#include "ThreadProcedure.h"

class CThread
{
public:
    enum Priority
    {
        IDLE = 0,               // Base priority level
        LOWEST,                 // 2 points below normal
        BELOW_NORMAL,           // 1 point below normal
        NORMAL,
        ABOVE_NORMAL,           // 1 point above normal
        HIGHEST,                // 2 points above normal
        TIME_CRITICAL,          // Absolute highest OS priority available

        NUM_PRIORITIES
    };

    enum Status
    {
        RUNNING     = 0,
        SUSPENDED,
        COMPLETE
    };

protected:
    CThread(ThreadProcedure* pkProcedure);

public:
    virtual ~CThread(void);

    //创建线程函数,线程创建后处于挂起状态
    static CThread* Create(ThreadProcedure* pkProcedure);

    //设置线程优先级
    bool         SetPriority(Priority ePriority);
    inline const Priority GetPriority() const {return m_ePriority;}
            
    //获取线程的执行状态和完成后的返回值            
    inline const Status GetStatus() const {return m_eStatus;}
    inline const unsigned int GetReturnValue() const {return m_uiReturnValue;}

    int  Suspend();
    int  Resume();
    bool WaitForCompletion();
    
public:
    //internel use!
    void* m_hThread;          //线程实例

    bool SystemCreateThread();
    bool SystemSetPriority(Priority ePriority);
    int  SystemSuspend();
    int  SystemResume();
    bool SystemWaitForCompletion();  //等待线程是否结束

    ThreadProcedure* m_pkProcedure;    //线程回调函数

    Priority        m_ePriority;
    volatile Status m_eStatus;
    volatile unsigned int m_uiReturnValue;
};

#endif  //#ifndef NITHREAD_H
