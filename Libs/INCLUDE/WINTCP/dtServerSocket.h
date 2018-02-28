#pragma once

#ifndef __DTSERVERSOCKET_H__
#define __DTSERVERSOCKET_H__

#include <vector>
#include <hash_map>

#include "TcpSocket.h"
#include "Base/Locker.h"
#include "Base/Log.h"
#include "Base/memPool.h"
#include "base/bitStream.h"

#define MAX_PACKET_COUNT 2048

struct stServerParam
{
	char Name[64];
	int MinClients;
	int MaxClients;
};

struct stOpParam
{
	int msg;      //the eOpParamType
	U64 val;	  //the value of eOpParamType to be set
};

enum eOpParamType
{
	IDLE_TIMEOUT,			//空闲超时
	CONNECT_TIMEOUT,		//连接超时
	CONNECT_TYPE,			
};

class dtServerSocket;
class dtServerSocketClient;
typedef void*(*NET_CALLBACK_FN)(dtServerSocket *pServer,dtServerSocketClient *pClient,int Msg,void *pData);

//the class is listen socket
class dtServerSocket : public ISocket
{
public:
	typedef CTcpSocket Parent;

	enum
	{
		MAX_COMPLETION_THREAD_NUM	= 1,		//单线程
		ADD_CLIENT_NUM				= 20,
		MAX_PENDING_SEND			= 50,
	};

	enum OP_CODE
	{
		OP_TIMETRACE,			 //跟踪
		OP_NORMAL,
		OP_QUIT,
		OP_RESTART,
		OP_DISCONNECT,
		OP_SEND,
		OP_ADDCLIENT,			
		OP_MAINTENANCE,
		OP_SETPARAM,		    //设置客户端信息
		
		OP_ON_CONNECT,
		OP_ON_DISCONNECT,
		OP_ON_PACKET,
	};

public:
	virtual bool Initialize(const char* ip, int port,void *param=NULL);
	virtual bool Start();
	virtual bool Stop ();
	virtual bool Restart()										{ return false;}				//不用实现
	virtual bool Send(const char *,int,int ctrlType)			{ return false;}				//不用实现
	virtual bool Connect()										{ return false;}				//不用实现
	virtual bool Disconnect(bool bForce)						{ return false;}				//不用实现

protected:
	virtual bool HandleReceive(OVERLAPPED_PLUS *,int ByteReceived)		{ return false;}		//不用实现
	virtual bool HandleConnect(OVERLAPPED_PLUS *,int ByteReceived)		{ return false;}		//不用实现
	virtual bool HandleSend(OVERLAPPED_PLUS *,int ByteSended)			{ return false;}		//不用实现

protected:
	virtual void OnNetFail(int nErrorCode,OVERLAPPED_PLUS *,int ByteSended){}
	virtual void OnDisconnect()									{}
	virtual void OnReceive(const char *pBuf, int nBufferSize)	{ return;		}				//不用实现
	virtual void OnConnect(int nErrorCode)						{}
	virtual void OnClear()										{}

private:
	static unsigned int WINAPI EventRoutine(LPVOID Param);
	static unsigned int WINAPI TimerRoutine(LPVOID Param);

public:
	friend class dtServerSocketClient;

	dtServerSocket(void);
	virtual ~dtServerSocket(void);

public:
	//状态检查
	bool IsShuttingDown()								{return m_bShuttingDown;};
	bool CanAccept()									{return m_bCanAccept;};
	int GetEnableClientNum()							{return m_nMaxClients-m_nClientCount;};

	void PostEvent(int Msg,void *pData=NULL);
	void PostEvent(int Msg,int id);
	void Send(int id,const char *,int,int ctrlType=0, bool bWebProtocl = false);
	void Send(int id,Base::BitStream &SendPacket,int ctrlType=0);
    void Send(Base::BitStream &SendPacket,int ctrlType=0);
    
    void DumpPackets(int onlineCount);
public:
	OVERLAPPED_PLUS*	GetBuffer(int SocketHandle,int iSize);
	void SetClientIdleTimeout(int SocketHandle,int idleTimeout);
	void SetClientConnectionTimeout(int SocketHandle,int ConnectionTimeout);
	void SetConnectionType(int SocketHandle,int Type);
	void SetNagle(bool flag) {m_bNagle = flag;}
	bool GetNagle() {return m_bNagle;}
	void CheckTimeEclipse();

protected:
	virtual ISocket *LoadClient	();
	bool AddClient				(int Num);
	int  AssignClientId			();
	dtServerSocketClient *GetClientById(int id);
	virtual int Maintenance		();

protected:
	CMemPool*			m_pMemPool;

	int			m_nClientCount;			//当前已经启用的客户端数量
	int			m_nMaxClients;			//最大可操作的客户端数量(最大监听数量)
	int			m_nMinClients;			//最小可操作的客户端数量
	volatile int	m_nIdSeed;

	bool		m_bShuttingDown;		//是否准备关闭标识
	bool		m_bCanAccept;			//是否允许接入连接
	bool		m_bNagle;

	int			m_seq;
protected:
	stdext::hash_map<int,dtServerSocketClient*>		m_ClientList;	//连接处理队列(inclued all the client info)
	dtServerSocketClient** m_ClientArray;	//连接队列(equal to the m_ClientList)

protected:
	//维护线程
	HANDLE		m_hAddClientEvent;
	HANDLE		m_hEndTimerEvent;
	HANDLE		m_hTimerThread;

	//完成端口和线程
	HANDLE		m_CompletionPort;  //IOCP
	HANDLE		m_CompletionThread[MAX_COMPLETION_THREAD_NUM];		//线程池
	int			m_CompletionThreadNum; //the num of IOCP Thread

    //监控参数
    DWORD		m_NetFNListTotalFreq[MAX_PACKET_COUNT];
    DWORD		m_NetFNListTotalSize[MAX_PACKET_COUNT];
public:
	char		m_Name[64];           // teh neame of server
};

#endif