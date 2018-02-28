#pragma once

#ifndef __ASYNSOCKET_H__
#define __ASYNSOCKET_H__

#include <list>
#include "Base/Locker.h"
#include "Base/Log.h"
#include "Base/memPool.h"
#include "base/bitStream.h"
#include "TcpSocket.h"

#define ASYNC_MAX_PENDINGSEND_NUM 1

struct stAsyncParam
{
	char Name[64];
};

//重叠IO类
class CAsyncSocket2 : public CTcpSocket
{
public:
	typedef CTcpSocket Parent;

public:
	bool Initialize(const char* ip, int port,void *pParam = NULL);
	bool Start();
	bool Stop ();
	bool Restart();
	bool Send(const char *,int,int ctrlType=PT_GUARANTEED);
	bool Connect();
	bool Disconnect(bool bForce=true);
    
    bool IsAllSended(void);
protected:
	bool HandleReceive(OVERLAPPED_PLUS *,int ByteReceived);
	bool HandleConnect(OVERLAPPED_PLUS *,int ErrorCode);
	bool HandleSend(OVERLAPPED_PLUS *,int ByteSended);

protected:
	void OnDisconnect()										{};
	void OnConnect(int nErrorCode=0)						{};

public:
	CAsyncSocket2();
	virtual ~CAsyncSocket2(void);

	bool Send(Base::BitStream &SendPacket,int ctrlType=PT_GUARANTEED);
	void OnNetFail(int nErrorCode,OVERLAPPED_PLUS *,int ByteSended);

	void pushSendList();
	void popSendList();
	void clearSendList();

	void SetMaxReceiveBufferSize(int MaxReceiveSize);

	void CheckTimeEclipse();

    U32  GetOutputBufferSize(void);
    U32  GetBackBufferSize(void);
protected:
	void SocketError(char *from,int error);
	bool CreateSocket();

	std::list<OVERLAPPED_PLUS* > m_OutBuffer;
	std::list<OVERLAPPED_PLUS* > m_BackBuffer;
	OVERLAPPED_PLUS* GetBuffer(int iSize);
	bool ReleaseBuffer(OVERLAPPED_PLUS*);
	
	static unsigned int WINAPI NetRoutine(LPVOID Param);
	static void CALLBACK ComplateRoutine(DWORD Error,DWORD Bytes,LPWSAOVERLAPPED Overlapped,DWORD InFlag);

public:
	CMyCriticalSection	m_cs;					/// 此类的锁

protected:
	sockaddr_in			m_LocalAddr;			/// 本机地址
	sockaddr_in			m_RemoteAddr;			/// 远程地址

	CMyCriticalSection	m_CritWrite;			/// outbuffer的临界锁
	CMyCriticalSection	m_CritFree;				/// 回收临界锁

	CMemPool*			m_pMemPool;

	bool				m_bPendingRead;			// a read is pending
	bool				m_bPushing;
    volatile U32        m_restartTime;

	char		m_Name[64];

	HANDLE				m_hSendEvent;
	HANDLE				m_hConnectEvent;
	HANDLE				m_hKillEvent;
	HANDLE				m_hThread;

	int						m_seq;
};

#endif


