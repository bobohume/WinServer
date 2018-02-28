
#include "wintcp/dtServerSocket.h"
#include "PlayerMgr.h"
#include "NetGate.h"

void CPlayerManager::ReleaseSocketMap(int SocketId, bool postEvent)
{
	CLocker lock(m_cs);

	SocketMapIter sit = m_SocketMap.find(SocketId);
	if (sit == m_SocketMap.end())
		return;
	m_AccountMap.erase(sit->second);
	m_SocketMap.erase(sit);
	if(postEvent)
		SERVER->GetUserSocket()->PostEvent(dtServerSocket::OP_RESTART, SocketId);
}

int CPlayerManager::GetAccountSocket(int AccountId)
{
	CLocker lock(m_cs);

	AccountMapIter ait = m_AccountMap.find(AccountId);
	if (ait != m_AccountMap.end())
		return ait->second.socketId;

	return 0;
}

int CPlayerManager::GetSocketAccount(int nSocket, T_UID& UID)
{
	CLocker lock(m_cs);

	SocketMapIter sit = m_SocketMap.find(nSocket);
	if (sit != m_SocketMap.end())
	{
		AccountMapIter ait = m_AccountMap.find(sit->second);
		if(ait != m_AccountMap.end())
		{
			UID = ait->second.UID;
			return ait->first;
		}
	}
	return 0;
}

ERROR_CODE CPlayerManager::AddAccountMap(int SocketId, T_UID UID, int AccountId)
{
	CLocker lock(m_cs);

	AccountMapIter ait = m_AccountMap.find(AccountId);
	if (ait != m_AccountMap.end())
	{
		if (ait->second.UID > UID)
			return GAME_UID_ERROR;

		ReleaseSocketMap(ait->second.socketId, ait->second.socketId!=SocketId);
	}
	m_AccountMap.insert(AccountMap::value_type(AccountId, stAccountInfo(UID, SocketId)));
	m_SocketMap[SocketId] = AccountId;
	return NONE_ERROR;
}

void CPlayerManager::TimeProcess(U32 bExit)
{
	//扫描所有的管理对象，超时的被认为是下线删除
	/*T_UID uid = 0;
	nId = SERVER->GetPlayerManager()->GetSocketAccount(pItem->Id, uid);

	if(nId)
		UserEventFn::NotifyWorldClientLost(nId, uid);

	g_Log.WriteLog( "disconnect incoming Socket:%d Account:%d UID:%d ",pItem->Id, nId,uid );

	// 相关清理
	SERVER->GetPlayerManager()->ReleaseSocketMap(pItem->Id);*/
	//return true;
}

void CPlayerManager::Broadcast(Base::BitStream& sendPacket,int ctrlType)
{
	CLocker lock(m_cs);

	for (SocketMapConIter it=m_SocketMap.begin(); it!=m_SocketMap.end(); ++it)
		SERVER->GetUserSocket()->Send(it->first, sendPacket,ctrlType);
}

void CPlayerManager::SendAccounts(void)
{
#ifdef _NTJ_UNITTEST_PRESS
	return;
#endif
    CLocker lock(m_cs);

    for (AccountMap::iterator iter = m_AccountMap.begin(); iter != m_AccountMap.end(); ++iter)
    {
        stAccountInfo& accountInfo = iter->second;

        char buf[64];
		Base::BitStream SendPacket(buf,sizeof(buf));
		stPacketHead* pHead = IPacket::BuildPacketHead(SendPacket,"GATE_WORLD_ACCOUNTRELINK",iter->first,SERVICE_WORLDSERVER);
		
        SendPacket.writeInt(accountInfo.UID,Base::Bit32);

		pHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
		SERVER->GetWorldSocket()->Send( SendPacket,PT_SYS );
    }
}