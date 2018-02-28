#ifndef REMOTE_PACKET_PROCESS_H
#define REMOTE_PACKET_PROCESS_H

#include "Common/CommonPacket.h"

using namespace Base;

class RemotePacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

	int		m_ConnectTimes;
public:
	RemotePacketProcess()
	{
		m_ConnectTimes = 0;
	}

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

_inline void RemotePacketProcess::OnConnect(int Error)
{
	if(!Error)
		SendClientConnect();
}

_inline void RemotePacketProcess::OnDisconnect()
{
	Parent::OnDisconnect();
	g_Log.WriteError("与远程帐号服务器断开");
}

_inline void RemotePacketProcess::HandleClientLogin()
{
	m_ConnectTimes++;

	if(m_ConnectTimes > 1)
		g_Log.WriteWarn("重新连接上远程帐号服务器");

	Parent::HandleClientLogin();

    RegisterServer(SERVICE_WORLDSERVER,0,SERVER->currentAreaId,m_ConnectTimes,0,0);
}

_inline void RemotePacketProcess::HandleClientLogout()
{
	Parent::HandleClientLogout();
}

_inline bool RemotePacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	if (SERVER->GetSocialManager()->IsValid(pHead->Message))
		SERVER->GetSocialWorkQueue()->PostEvent(m_pSocket->GetClientId(), pHead, iSize, true);
	else
		SERVER->GetWorkQueue()->PostEvent(m_pSocket->GetClientId(), pHead, iSize, true);

	return true;
}

#endif

