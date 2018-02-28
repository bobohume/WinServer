#include "ServerMgr.h"
#include "ChatServer.h"
#include "WorldServer\LockCommon.h"

void CServerMgr::addServer(int socketId,int lineId,int gateId)
{
	OLD_DO_LOCK(m_cs);
    
    if (lineId >= MAX_LINE_COUNT || gateId >= MAX_GATE_COUNT)
    {
        g_Log.WriteError("line[%d]或gate[%d]超过最大值",lineId,gateId);
        return;
    }

	GateServer* pServer = &m_gates[lineId][gateId];
    
	pServer->socketId = socketId;
	pServer->lineId   = lineId;
	pServer->gateId   = gateId;
	
	mServerMap[socketId] = pServer;
}

void CServerMgr::removeServer(int socketId)
{
	OLD_DO_LOCK(m_cs);

	ServerMap::iterator iter = mServerMap.find(socketId);

    if(iter == mServerMap.end())
        return;

    GateServer* pServer = iter->second;
    pServer->Clear();

	mServerMap.erase(socketId);
}

int CServerMgr::GetServer(int lineId,int gateId)
{
    OLD_DO_LOCK(m_cs);

    if (lineId >= MAX_LINE_COUNT || gateId >= MAX_GATE_COUNT)
    {
        return 0;
    }

    GateServer* pServer = &m_gates[lineId][gateId];
    return pServer->IsValid() ? pServer->socketId : 0;
}

void CServerMgr::getGates(int* gates, int& count)
{
	OLD_DO_LOCK(m_cs);

	int i = 0;

	for(ServerMap::iterator iter = mServerMap.begin(); iter != mServerMap.end() && i < count; iter++,i++)
	{
		gates[i] = iter->first;
	}

	count = i;
}

void CServerMgr::getAllGates(std::vector<int>& gateList,int lineId)
{
	OLD_DO_LOCK(m_cs);

    if (lineId >= MAX_LINE_COUNT)
        return;

    for(int i = 0; i < MAX_GATE_COUNT; ++i)
    {
        GateServer* pServer = &m_gates[lineId][i];

        if (!pServer->IsValid())
            continue;

        gateList.push_back(pServer->socketId);
    }
}

int CServerMgr::getLineId( int socketId )
{
	OLD_DO_LOCK(m_cs);

	ServerMap::iterator iter = mServerMap.find(socketId);

	if(iter == mServerMap.end())
		return 0;
	
	return iter->second->lineId;
}

CServerMgr::CServerMgr()
{

}

CServerMgr::~CServerMgr()
{

}

//接受NetGate注册
ServerEventFunction( CChatServer, HandleRegisterRequest, "COMMON_RegisterRequest" )
{
    int type   = pPack->readInt(Base::Bit8);
    U32 lineId = pPack->readInt(Base::Bit8);
    U32 gateId = pPack->readInt(Base::Bit32);
    pPack->readInt(Base::Bit32);

    int IP = 0,Port = 0;

    if(pPack->readFlag())
    {
    }

    SERVER->getServerMgr()->addServer(socketId,lineId,gateId);

    switch(type)
    {
    case SERVICE_GATESERVER:
        {
            char Buffer[64];
            Base::BitStream SendPacket(Buffer,sizeof(Buffer));
            stPacketHead *pSendHead = IPacket::BuildPacketHead(SendPacket,"COMMON_RegisterResponse");

            pSendHead->PacketSize = SendPacket.getPosition()-IPacket::GetHeadSize();
            SERVER->getServerSocket()->Send(socketId,SendPacket);
        }
        break;
    }

    return true;
}

//接受WorldServer的注册响应
ServerEventFunction(CChatServer,HandleRegisterResponse,"COMMON_RegisterResponse")
{
	return true;
}