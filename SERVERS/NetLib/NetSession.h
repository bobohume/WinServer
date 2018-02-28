#pragma once

class NETLIB_CLASS CNetSession
{
public:
	typedef unsigned long ID_HANDLE;
	static const int SERVER_ID_HANDLE = 0;

	enum ProtocolType
	{
		CONNECT,
		TICK,
		LOGIC,
		DISCONNECT,
	};

	SOCKADDR_IN* getAddress();

	CNetSession();
	virtual ~CNetSession();

	// 连接
	virtual void onConnected()	{;}
	
	// 超时
	virtual void onTimeout()	{;}

	// 断开连接
	virtual void onDisconnect()	{;}

	// 心跳
	virtual void onTick()		{;}

public:
	ID_HANDLE getId();
	DWORD LastTickTime;

private:
	ID_HANDLE m_dwId;
	SOCKADDR_IN mSockaddr;

    static int ms_id;
};