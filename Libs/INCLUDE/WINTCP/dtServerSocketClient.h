#pragma once

#ifndef __DTSERVERSOCKETCLIENT_H__
#define __DTSERVERSOCKETCLIENT_H__

#include <list>

#include "ISocket.h"
#include "dtServerSocket.h"
#include "Base/Locker.h"
#include "Base/Log.h"
#include "Base/memPool.h"
#include "WebSocketProtocol.h"


//the class include the accept client socket
class dtServerSocketClient : public CTcpSocket
{
	typedef CTcpSocket Parent;
public:
	virtual bool Initialize(const char* ip, int port,void *)	{ return true ;}
	virtual bool Start();
	virtual bool Stop ()										{ return false;}				//不用实现
	virtual bool Restart();
	virtual bool Send(const char *,int,int ctrlType=0);
	virtual bool Connect()										{ return false;}				//不用实现
	virtual bool Disconnect(bool bForce);

    sockaddr_in  GetRemoteAddr(void)							{return m_RemoteAddr;}
protected:
	virtual bool HandleReceive(OVERLAPPED_PLUS *,int ByteReceived);
	virtual bool HandleConnect(OVERLAPPED_PLUS *,int ByteReceived);
	virtual bool HandleSend(OVERLAPPED_PLUS *,int ByteSended);

protected:
	virtual void OnDisconnect();
	//virtual void OnReceive(const char *pBuf, int nBufferSize)	{ return ;		};
	virtual void OnConnect(int nErrorCode);
	virtual void OnNetFail(int nErrorCode,OVERLAPPED_PLUS *,int ByteSended);

public:
	friend class dtServerSocket;


protected:
	SOCKET				m_nListenSocket;			// 监听端口(the socket to server)

	sockaddr_in			m_LocalAddr;				// 本地地址
	sockaddr_in			m_RemoteAddr;				// 远程地址

	dtServerSocket*		m_pServer;					/// Pointer to the server that this client belongs to.			
	HANDLE				m_CompletionPort;			/// Pointer to IOCP
	int					m_CompletionThreadNum;

	bool				m_bPendingRead;				/// a read is pending

	std::list<OVERLAPPED_PLUS*>	m_SendingList;      /// the list first in first out

	OVERLAPPED_PLUS		*m_pAcceptBuf;

protected:
	CMemPool*			m_pMemPool;                /// dynamic alloc OVERLAPPED

	//内存管理
	OVERLAPPED_PLUS*	GetBuffer			(int iSize);

	bool				BeginSend			(OVERLAPPED_PLUS *lpOverlapped,int ctrlType=0);       //ready to send packet
	void				SetListenSocket		(SOCKET sdListen);
	void				SetIoCompRoutine	(HANDLE CompletionPort,int ThreadNum);
	void				SetServer			(dtServerSocket *pServer);
	
	bool				_DoSend				();

	void				ResetVar			();

public:
						dtServerSocketClient(void);
	virtual				~dtServerSocketClient(void);
	CWebSocketProtocol*  m_pWebSocketProtocol;

public:
	int					GetClientId			()														{return m_ClientId;}

	BOOL ReleaseBuffer(OVERLAPPED_PLUS*);
	void PostEvent(int Msg,void *pData=NULL);

	SERVER_STATE_FLAGS GetState()
	{ 
		return m_nState;	
	};

	int CheckTimeout(bool isRestartable);

	int GetConnectTime();
	int GetIdleTime();
	char *GetConnectedIP()
	{
		return m_sIP;
	}

	int GetConnectedIPi()
	{
		return (int)m_RemoteAddr.sin_addr.s_addr;
	}

};

#endif