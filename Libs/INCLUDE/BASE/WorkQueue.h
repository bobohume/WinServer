#pragma once
#ifndef _WORK_QUEUE_H
#define _WORK_QUEUE_H

#include <process.h>
#include "base/memPool.h"
#include "base/Log.h"
#include "timeSignal.h"

class SEH_Base
{
public:
	SEH_Base(unsigned int code, PEXCEPTION_POINTERS EP) 
	{
		setExceptInfo(code, EP);
	}

	SEH_Base(const SEH_Base& other)
	{
		*this = other;
	}

	SEH_Base& operator=(const SEH_Base& other)
	{
		setExceptInfo(other.mCode, &other.mEP);
		return *this;
	}

	unsigned int getCode() { return mCode; }
	PEXCEPTION_POINTERS getExceptionPointer() { return &mEP; }

protected:
	void setExceptInfo(unsigned int code, const EXCEPTION_POINTERS* EP)
	{
		mCode = code;
		mException = *EP->ExceptionRecord;
		mContext = *EP->ContextRecord;
		mEP.ExceptionRecord = &mException;
		mEP.ContextRecord = &mContext;
	}

	unsigned int mCode;
	CONTEXT mContext;
	EXCEPTION_RECORD mException;
	EXCEPTION_POINTERS mEP;
};

class SHException 
	: public SEH_Base
{
public:
	SHException(unsigned int code, PEXCEPTION_POINTERS EP)
		:SEH_Base(code, EP) {}
};

inline void Exception(unsigned int code, PEXCEPTION_POINTERS EP)
{
	throw SHException(code, EP);
}

#define SET_SHE_HANDLER _set_se_translator((_se_translator_function)&Exception);

enum WORKQUEUE_MESSGAE
{
	WQ_STARTED,
	WQ_TIMETRACE,
	WQ_CONNECT,
	WQ_DISCONNECT,
	WQ_PACKET,
	WQ_PACKET_SERVER,
	WQ_PACKET_CLIENT,
	WQ_LOGIC,
	WQ_COMMAND,
	WQ_TIMER,
	WQ_QUIT,

	WQ_ORG,
	WQ_SOCIAL,
	WQ_NORMAL,

	WQ_CONFIGMONITOR,
	WQ_PROFILEFRAME,
	WQ_SCRIPT,
};

typedef int (*WORK_QUEUE_FN)(LPVOID Param);

struct WorkQueueItemStruct
{
	int					Id;
	WORKQUEUE_MESSGAE	opCode;
	int					size;
	char *				Buffer;

#ifdef _TIME_TEST_
	U64				time;
#endif
};

class CWorkQueue
{
	int						m_seq;
protected:
	HANDLE					m_QueueHandle;
	HANDLE					*m_ThreadHandle;
	int						m_ThreadNum;
	WORK_QUEUE_FN			m_Callback;
	int						m_WaitTime;
	bool					m_Stop;
	TimeSignal<U32>			m_TimerMgr;

public:
	bool					m_WorkForLog;
	char					m_Name[64];

public:
	CWorkQueue(bool workForLog = false)
	{
		m_QueueHandle	= NULL;
		m_ThreadHandle	= NULL;
		m_Callback		= NULL;
		m_WaitTime      = INFINITE;
		m_ThreadNum		= 0;
		m_Stop			= true;
		m_WorkForLog	= workForLog;
		m_seq			= 0;
	}

	~CWorkQueue()
	{
		if(!m_Stop)
			Stop();
	}

	bool Initialize(WORK_QUEUE_FN fn,int ThreadNum,const char*Name, int iTimeOut)
	{
		strcpy_s(m_Name,64,Name);

		m_Callback = fn;
		if(!m_Callback)
			return false;

		m_WaitTime = iTimeOut;

		m_QueueHandle = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)this, 0);
		if(!m_QueueHandle)
		{
			if(!m_WorkForLog)
			{
				g_Log.WriteError("workQueue [%s] create fail",m_Name);
			}
			else
			{
				printf("workQueue [%s] create fail",m_Name);
			}

			return false;
		}

		ThreadNum = 1;
		m_ThreadHandle = new HANDLE[ThreadNum];

		m_ThreadNum = ThreadNum;
		unsigned int threadID = 0;
		for (int i=0; i < m_ThreadNum; i++ )
		{
			m_ThreadHandle[i] = (HANDLE)_beginthreadex( NULL, 0, WorkRoutine, this, 0, &threadID );
		}

		m_Stop = false;
		return true;
	}

	void CheckTimeEclipse()
	{
		PostEvent(m_seq++,NULL,0,false,WQ_TIMETRACE);
	}

	TimeSignal<U32>& GetTimerMgr(){return m_TimerMgr;}

	bool PostEvent(int Id,void *data,int size,bool CopyData=false,WORKQUEUE_MESSGAE QueueMsg=WQ_PACKET)
	{
		if(m_Stop || !CMemPool::GetInstance())
		{
			if(!CopyData)
				MEMPOOL_FREE((MemPoolEntry)data);

			return false;
		}

		WorkQueueItemStruct *pStruct = (WorkQueueItemStruct *)MEMPOOL_ALLOC(sizeof(WorkQueueItemStruct));
		if(!pStruct)
		{
			if(!CopyData)
				MEMPOOL_FREE((MemPoolEntry)data);
			return false;
		}

		if(CopyData)
		{
			pStruct->Buffer = (char *)MEMPOOL_ALLOC(size);
			if(!pStruct->Buffer)
			{
				MEMPOOL_FREE((MemPoolEntry)pStruct);
				return false;
			}

			memcpy(pStruct->Buffer,data,size);
		}
		else
		{
			pStruct->Buffer = (char *)data;
		}

		pStruct->Id		= Id;
		pStruct->size	= size;
		pStruct->opCode = QueueMsg;

#ifdef _TIME_TEST_
		//if(QueueMsg == WQ_TIMETRACE)
		//{

		//}


		pStruct->time	= GetTickCount64();
#endif



		BOOL ret = PostQueuedCompletionStatus(m_QueueHandle,0,(int)QueueMsg,(LPOVERLAPPED)pStruct);
		if(!ret)
		{
			if(!CopyData)
				MEMPOOL_FREE((MemPoolEntry)data);
			else
				MEMPOOL_FREE((MemPoolEntry)pStruct->Buffer);

			MEMPOOL_FREE((MemPoolEntry)pStruct);

			int err = GetLastError();
			char errstr[128];
			sprintf_s(errstr,sizeof(errstr),"work Queue [%s] PostQueuedCompletionStatus fail err=%d",m_Name,err);

			if(!m_WorkForLog)
			{
				g_Log.WriteError(errstr);
			}
			else
			{
				printf(errstr);
			}

			return false;
		}
		return true;
	}

	void Stop()
	{
		m_Stop = true;

		for(int i=0;i<m_ThreadNum;i++)
			PostQueuedCompletionStatus(m_QueueHandle,0,(int)WQ_QUIT,NULL);

		WaitForMultipleObjectsEx(m_ThreadNum,m_ThreadHandle,TRUE,10000,FALSE);

		for(int i=0;i<m_ThreadNum;i++)
			CloseHandle(m_ThreadHandle[i]);
		delete [] m_ThreadHandle;

		CloseHandle(m_QueueHandle);
	}

	static unsigned int WINAPI WorkRoutine(LPVOID Param)
	{
		CWorkQueue *pQueue = (CWorkQueue*)Param;
		DWORD		dwByteCount=0,dwErrorCode=0,dwCode=0;
		WorkQueueItemStruct		*pData = NULL;

		SET_SHE_HANDLER;

		while(1)
		{
			if(!::GetQueuedCompletionStatus(pQueue->m_QueueHandle, &dwByteCount, (ULONG_PTR *)&dwCode,(LPOVERLAPPED*)&pData, pQueue->m_WaitTime))
			{
				DWORD dwErrorCodeBak = GetLastError();
				if(dwErrorCodeBak == WAIT_TIMEOUT)
				{
					pQueue->GetTimerMgr().trigger();
					continue;
				}

				if(!pQueue->m_WorkForLog)
				{
					g_Log.WriteError("workqueue GetQueuedCompletionStatus err=%d",dwErrorCodeBak);
				}
				else
				{
					printf("workqueue GetQueuedCompletionStatus err=%d",dwErrorCodeBak);
				}

				continue;
			}

			if(dwCode == (int)WQ_QUIT)
			{
				if(!pQueue->m_WorkForLog)
				{
					g_Log.WriteLog(std::string("handle WQ_QUIT,workQueue exit"));
				}
				else
				{
					printf("handle WQ_QUIT,workQueue exit");
				}

				return 0;
			}

			if( !pData )
			{  //Ray: 有可能是来不及处理，而主线程等待超时后直接把端口关闭造成的错误，会引起关闭时刻内存泄漏
				int err = GetLastError();
				char errstr[128];
				sprintf_s(errstr,sizeof(errstr),"work Queue GetQueuedCompletionStatus fail err=%d",err);

				if(!pQueue->m_WorkForLog)
				{
					g_Log.WriteError(errstr);
				}
				else
				{
					printf(errstr);
				}
				return err;
			}

#ifdef _TIME_TEST_
			U64 eclapse = GetTickCount64() - pData->time;
			if(eclapse > 19 && strcmp(pQueue->m_Name,"g_Log"))  //Ray: 0就不打了
			{
				if(!pQueue->m_WorkForLog)
				{
					g_Log.WriteLog("Id=%d [%s] 处理速度 %d tick",pData->Id,pQueue->m_Name,eclapse);
				}
				else
				{
					printf("Id=%d [%s] 处理速度 %d tick",pData->Id,pQueue->m_Name,eclapse);
				}
			}
#endif
			//if(dwCode == (int)WQ_TIMETRACE)
			//{
			//	int eclapse = GetTickCount() - pData->size;
			//	if(eclapse > 19)  //Ray: 0就不打了
			//	{
			//		if(!pQueue->m_WorkForLog)
			//		{
			//			g_Log.WriteLog("Id=%d [%s] 处理速度 %d tick",pData->Id,pQueue->m_Name,eclapse);
			//		}
			//		else
			//		{
			//			printf("Id=%d [%s] 处理速度 %d tick",pData->Id,pQueue->m_Name,eclapse);
			//		}
			//	}
			//}
			//else
			//{
			pQueue->m_Callback(pData);
			if(pQueue->m_WaitTime != INFINITE)
				pQueue->GetTimerMgr().trigger();
			//}

			MEMPOOL_FREE((MemPoolEntry)pData->Buffer);
			MEMPOOL_FREE((MemPoolEntry)pData);
		}

		if(!pQueue->m_WorkForLog)
		{
			g_Log.WriteLog(std::string("workQueue thread termination"));
		}
		else
		{
			printf("workQueue thread termination");
		}
	}
};

#endif
