#ifndef CHAT_CLIENT_PACKET_PROCESS_H
#define CHAT_CLIENT_PACKET_PROCESS_H

#include "Common/CommonPacket.h"

using namespace Base;

class ChatClientPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

	int		m_ConnectTimes;
public:
	ChatClientPacketProcess()
	{
		m_ConnectTimes = 0;
#ifdef _NTJ_UNITTEST_PRESS
		m_recvnum = 0;
#endif
		g_Log.WriteWarn("等待连接聊天服务器");
	}
#ifdef _NTJ_UNITTEST_PRESS
	int m_recvnum;
#endif
	void OnConnect(int Error=0);
	void OnDisconnect();
protected:
	bool HandleGamePacket(stPacketHead *pHead,int iSize);
	void HandleClientLogin();
	void HandleClientLogout();

	//消息处理

	//消息发送

	//数据库操作
};

_inline void ChatClientPacketProcess::OnConnect(int Error)
{
	if(!Error)
		SendClientConnect();
}

_inline void ChatClientPacketProcess::OnDisconnect()
{
	Parent::OnDisconnect();
	if(m_ConnectTimes>=1)
	{
		g_Log.WriteError("与聊天服务器断开");
	}
}

_inline void ChatClientPacketProcess::HandleClientLogin()
{
	m_ConnectTimes++;

	int nNetLineId = atoi( NetLineId.c_str() );
	int nNetGateId = atoi( NetGateId.c_str() );
	const char* szIp = UserNetIP.c_str();
	int port = atoi( UserNetPort.c_str() );
	RegisterServer(SERVICE_GATESERVER,nNetLineId,nNetGateId,m_ConnectTimes, szIp, port );

	if(m_ConnectTimes>1)
	{
		g_Log.WriteWarn("重新连接上聊天服务器");
	}
	else
	{
		g_Log.WriteLog("连接聊天服务器成功!");
	}

	Parent::HandleClientLogin();
}

_inline void ChatClientPacketProcess::HandleClientLogout()
{
	Parent::HandleClientLogout();
}

_inline bool ChatClientPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
#ifdef _NTJ_UNITTEST_PRESS
	// 	extern CWorkQueue * g_workqueue;
	// 	g_workqueue->PostEvent(0,pHead,iSize,true);
	extern int g_testrecv;
	extern std::vector<int> g_msgRecvNum;
	if(pHead->Message > MSG_MAIL_BEGIN&&pHead->Message<MSG_MAIL_END&&pHead->DestServerType == SERVICE_ZONESERVER)
	{
		g_msgRecvNum[0]++;
		m_recvnum ++;
	}

#else
	SERVER->GetWorkQueue()->PostEvent(-2,pHead,iSize,true);
#endif
	return true;
}

#endif

