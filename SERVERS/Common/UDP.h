
#pragma once
#include "base/Locker.h"
#include "base/Log.h"
#include <WinSock2.h>
#include "Base/memPool.h"
#include "wintcp//iSocket.h"

#define MAX_PENDING_READ_NUM	1
#define UDP_SERVER_WORK_BUFFER_SIZE 30000
#define HEAD_RESERVE 100

struct uRawAddress
{
	int ip;    //ip
	int port;  //port or playerId (ip+playerId = Fixed id string for relink)
	int id;	   //socketHandle
};

enum
{
	MAX_COMPLETION_THREAD_NUM	= 1,		//单线程
};

enum OP_CODE
{
	OP_TIMETRACE,
	OP_SEND,
	OP_NORMAL,
	OP_QUIT,
};

struct UDP_OVERLAPPED_PLUS : public WSAOVERLAPPED
{
	SOCKADDR_IN addr;
	int addr_len;
	char		bCtxWrite;
	WSABUF		WsaBuf;
};

class CUDPServer
{
	CMemPool*			m_pMemPool;

public:
    enum UDP_SIDE
    {
        UDP_SIDE_CLIENT,
        UDP_SIDE_ZONE,
    };

	CUDPServer(const char *ip,int nPort,int side);
	virtual ~CUDPServer(void);

	virtual BOOL Send(const char *,int,int ,int);
	void SocketError(char *from,int error);

	virtual BOOL Start();
	virtual BOOL Stop();
	BOOL Create();

public:
	BOOL BeginSend(UDP_OVERLAPPED_PLUS *lpOverlapped);
	BOOL HandleSend(UDP_OVERLAPPED_PLUS *lpOverlapped,int ByteReceived);
	BOOL HandleReceive(UDP_OVERLAPPED_PLUS *lpOverlapped,int ByteReceived);
	void PostEvent(int Msg,void *pData);
	virtual BOOL OnReceive(char *buf,int nByte,SOCKADDR_IN *addr);
	BOOL OnNetFail(int dwErrorCode,UDP_OVERLAPPED_PLUS *ov,int dwByteCount);

public:
	void SetMaxWorkBufferSize(size_t MaxBuffSize) {m_MaxWorkBufferSize = MaxBuffSize;}
	size_t GetMaxWorkBufferSize(){ return m_MaxWorkBufferSize;}

	void SetOutServer(CUDPServer *pOut) {m_pOutServer = pOut;}

	int DumpSendPackets(){int count = m_SendCount; m_SendCount = 0; return count;}
	int DumpSendErrPackets(){int count = m_SendErr; m_SendErr = 0; return count;}
	int DumpRecvPackets(){int count = m_RecvCount; m_RecvCount = 0; return count;}
	int DumpRecvErrPackets(){int count = m_RecvErr; m_RecvErr = 0; return count;}

	void CheckTimeEclipse();

public:
	int					m_MaxWorkBufferSize;
	SOCKET				m_sdClient;
	int					m_nPort;
	char				m_sIP[64];
    UDP_SIDE            m_side;

	int					m_PendingNum;

	int					m_SendCount;
	int					m_SendErr;
	int					m_RecvCount;
	int					m_RecvErr;
	int					m_seq;

	CUDPServer *		m_pOutServer;

protected:
	UDP_OVERLAPPED_PLUS* GetBuffer(int nSize);
	BOOL CUDPServer::ReleaseBuffer(UDP_OVERLAPPED_PLUS *pBuf);

public:

	HANDLE		m_hEndTimerEvent;
	HANDLE		m_hTimerThread;

	//完成端口和线程
	HANDLE		m_CompletionPort;
	HANDLE		m_CompletionThread[MAX_COMPLETION_THREAD_NUM];		//线程池
	int			m_CompletionThreadNum;

	static unsigned int WINAPI NetRoutine(LPVOID pParam);
};

