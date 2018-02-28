#ifndef WORLD_CLIENT_PACKET_PROCESS_H
#define WORLD_CLIENT_PACKET_PROCESS_H

#include "Common/CommonPacket.h"
#ifdef _NTJ_UNITTEST_PRESS
#include "Common/PlayerStructEx.h"
#endif
#include "PlayerMgr.h"

using namespace Base;

class WorldClientPacketProcess : public CommonPacket
{
	typedef CommonPacket Parent;

	int		m_ConnectTimes;
public:
	WorldClientPacketProcess()
	{
		m_ConnectTimes = 0;
#ifdef _NTJ_UNITTEST_PRESS
		m_recvnum = 0;
#endif
		g_Log.WriteWarn("等待连接世界服务器");
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

_inline void WorldClientPacketProcess::OnConnect(int Error)
{
	if(!Error)
    {
		SendClientConnect();
    }
}

_inline void WorldClientPacketProcess::OnDisconnect()
{
	Parent::OnDisconnect();

	if(m_ConnectTimes>=1)
	{
		g_Log.WriteError("与世界服务器断开");
	}
}

_inline void WorldClientPacketProcess::HandleClientLogin()
{
	m_ConnectTimes++;

	int nNetLineId = atoi( NetLineId.c_str() );
	int nNetGateId = atoi( NetGateId.c_str() );
	const char* szIp = UserNetIP.c_str();
	int port = atoi( UserNetPort.c_str() );

	RegisterServer(SERVICE_GATESERVER,nNetLineId,nNetGateId,m_ConnectTimes, szIp, port,(UserNetIP2 != "") ? UserNetIP2.c_str() : 0);

	if(m_ConnectTimes>1)
	{
		g_Log.WriteWarn("重新连接上世界服务器");
	}
	else
	{
		g_Log.WriteLog("连接世界服务器成功!");
	}
    //发送当前所有帐号信息到world,用来重新绑定world上的帐号对应gate的连接
    SERVER->GetPlayerManager()->SendAccounts();

	Parent::HandleClientLogin();
}

_inline void WorldClientPacketProcess::HandleClientLogout()
{
	Parent::HandleClientLogout();
}

_inline bool WorldClientPacketProcess::HandleGamePacket(stPacketHead *pHead,int iSize)
{
	SERVER->GetWorkQueue()->PostEvent(-1,pHead,iSize,true);
	return true;
}

#endif

