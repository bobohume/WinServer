#include "DBLib/dbLib.h"
#include "Base/Log.h"
#include "wintcp/dtServerSocket.h"
#include "Common/PacketType.h"
#include "ServerMgr.h"
#include "WorldServer.h"
#include "PlayerMgr.h"
#include "base/WorkQueue.h"
#include "Common/DumpHelper.h"
#include "DBLayer\Data\TBLServerMap.h"
#include "DBLayer/Data/TBLWorld.h"
#include "EventProcess.h"
#include "wintcp/AsyncSocket.h"
#include "Common/CfgBuilder.h"

using namespace std;

CServerManager::CServerManager()
{
	m_pAccountInfo	  = NULL;
	m_pChatServerInfo = NULL;
	m_pWorldDBInfo    = 0;
    m_openTime        = 0;
}

CServerManager::~CServerManager()
{
    OLD_DO_LOCK(m_cs);

	if(m_pAccountInfo)
		delete m_pAccountInfo;

	if( m_pChatServerInfo )
		delete m_pChatServerInfo;

	if (m_pWorldDBInfo)
		delete m_pWorldDBInfo;

    m_pAccountInfo    = 0;
    m_pChatServerInfo = 0;
    m_pWorldDBInfo    = 0;
}

void CServerManager::AddServerMap(stServerInfo &ServerInfo)
{
	OLD_DO_LOCK(m_cs);

	HashSocketMap::iterator sit = m_SocketMap.find(ServerInfo.SocketId);

	if(sit != m_SocketMap.end())
		delete sit->second;

	stServerInfo *pServerInfo = new stServerInfo;
	*pServerInfo = ServerInfo;
	m_SocketMap[ServerInfo.SocketId] = pServerInfo;

	switch(ServerInfo.Type)
	{
	case SERVICE_ACCOUNTSERVER:
        {
		    m_pAccountInfo = pServerInfo;
        }
		break;
	case SERVICE_GATESERVER:
		{
			m_LineGateMap[ServerInfo.LineId][ServerInfo.GateId] = pServerInfo;

			in_addr ia;
			ia.s_addr = pServerInfo->Ip[0];
			g_Log.WriteLog("ADD GATE SERVER: [%d]-[%d]-[%s:%d]", pServerInfo->LineId, pServerInfo->GateId, inet_ntoa( ia ), ntohs( pServerInfo->Port ) );
		}
		break;
	case SERVICE_DATAAGENT:
		m_DataAgentMap[ServerInfo.SocketId] = ServerInfo;
        g_Log.WriteLog("Add DataAgent line[%d] socket[%d]",ServerInfo.LineId,ServerInfo.SocketId);

		break;
	case SERVICE_CHATSERVER:
        {
            m_pChatServerInfo = pServerInfo;
        }
		break;
	case SERVICE_WORLDDB:
		m_pWorldDBInfo = pServerInfo;
		break;
	}
}

void CServerManager::ReleaseServerMap(int SocketId,bool postEvent)
{
	OLD_DO_LOCK(m_cs);

	HashSocketMap::iterator sit = m_SocketMap.find(SocketId);

	if(sit == m_SocketMap.end())
		return;

	stServerInfo* pServerInfo = sit->second;

    g_Log.WriteLog("服务器断开连接socketid[%d]",SocketId);
    
	switch(pServerInfo->Type)
	{
	case SERVICE_ACCOUNTSERVER:
        {
            g_Log.WriteLog("与Account服务器断开连接");

            if (m_pAccountInfo && SocketId != m_pAccountInfo->SocketId)
            {
                g_Log.WriteError("account socket id missmatched id[%d]",m_pAccountInfo->SocketId);    
            }
            else
            {
		        m_pAccountInfo = NULL;
            }
        }
		break;
    case SERVICE_GATESERVER:
        {
            g_Log.WriteLog("与Gate服务器断开连接,line[%d] id[%d]",pServerInfo->LineId,pServerInfo->GateId);

            stServerInfo* pGateServer = 0;
            HashSocketMap::iterator iterFinded = m_LineGateMap[pServerInfo->LineId].find(pServerInfo->GateId);
            
            if (iterFinded != m_LineGateMap[pServerInfo->LineId].end())
            {
                pGateServer = iterFinded->second;
            }

            if (pGateServer && SocketId != pGateServer->SocketId)
            {
                g_Log.WriteError("gate socket id missmatched id[%d]",pGateServer->SocketId); 
            }
            else
            {
                m_LineGateMap[pServerInfo->LineId].erase(pServerInfo->GateId);

                if(m_LineGateMap[pServerInfo->LineId].size() == 0)
                    m_LineGateMap.erase(pServerInfo->LineId);
            }
        }
		break;
	case SERVICE_DATAAGENT:
        {
            g_Log.WriteLog("与DataAgent服务器断开连接,line[%d]",pServerInfo->LineId);
		    m_DataAgentMap.erase( SocketId );
        }
		break;
	case SERVICE_CHATSERVER:
        {
            g_Log.WriteLog("与Chat服务器断开连接");

            if (m_pChatServerInfo && SocketId != m_pChatServerInfo->SocketId)
            {
                g_Log.WriteLog("chat socket id missmatched id[%d]",m_pChatServerInfo->SocketId);    
            }
            else
            {
		        m_pChatServerInfo = NULL;
            }
        }
		break;
	case SERVICE_WORLDDB:
        {
            g_Log.WriteLog("与WorldDB服务器断开连接");
        
            if (m_pWorldDBInfo && SocketId != m_pWorldDBInfo->SocketId)
            {
                g_Log.WriteLog("worlddb socket id missmatched id[%d]",m_pWorldDBInfo->SocketId);    
            }
            else
            {
		        m_pWorldDBInfo = NULL;
            }
        }

		break;
	}

	m_SocketMap.erase(pServerInfo->SocketId);
	delete pServerInfo;

	if(postEvent)
    {
		SERVER->GetServerSocket()->PostEvent(dtServerSocket::OP_RESTART,SocketId);
    }
}

int CServerManager::GetServerSocket(int Type,int LineId,int GateId)
{
    OLD_DO_LOCK(m_cs);

    stServerInfo* pServerInfo = GetServerInfo(Type,LineId,GateId);
    
    if (0 == pServerInfo)
        return 0;

    return pServerInfo->SocketId;
}

stServerInfo* CServerManager::GetServerInfo(int Type,int LineId,int GateId)
{
	switch(Type)
	{
	case SERVICE_ACCOUNTSERVER:
        {
		    return m_pAccountInfo;
        }
        break;
	case SERVICE_GATESERVER:
		{
			HashLineGateMap::iterator slit = m_LineGateMap.find(LineId);

			if(slit == m_LineGateMap.end())
            {       
				return NULL;
            }

			HashSocketMap::iterator sit = slit->second.find(GateId);
			if(sit == slit->second.end())
				return NULL;

			return sit->second;
		}
	case SERVICE_CHATSERVER:
		{
			return m_pChatServerInfo;
		}
		break;
	case SERVICE_WORLDDB:
		{
			return m_pWorldDBInfo;
		}
		break;
	}

	return NULL;
}

bool CServerManager::GetGateInfoBySocket(int SocketId,U32& lineId,U32& gateId)
{
	OLD_DO_LOCK(m_cs);

	HashSocketMap::iterator iter = m_SocketMap.find(SocketId);

	if(iter == m_SocketMap.end() || 0 == iter->second)
		return false;

    lineId = iter->second->LineId;
    gateId = iter->second->GateId;

	return true;
}

// ----------------------------------------------------------------------------
int CServerManager::GetLineId(int SocketId)
{
	OLD_DO_LOCK(m_cs);

	HashSocketMap::iterator sit = m_SocketMap.find(SocketId);

	if(sit == m_SocketMap.end())
	{
		return 0;
	}

	return sit->second->LineId;
}

bool CServerManager::Initialize()
{
	OLD_DO_LOCK(m_cs);

	//bool ret  = readMapConfig();
	bool ret1 = readWorldSetting();

	return  ret1;
}

/*bool CServerManager::readMapConfig()
{
	m_ZoneInfoList.clear();

	std::vector<TBLServerMap::stZoneInfo> ZoneInfo;
	TBLServerMap tb_servermap(SERVER_CLASS_NAME::GetActorDB());
	DBError err = tb_servermap.Load(ZoneInfo);
	if(err != DBERR_NONE)
		return false;

	stZoneInfo Info;
	for(size_t i = 0; i < ZoneInfo.size(); ++i)
	{
		Info.ZoneId			= ZoneInfo[i].ZoneId;
		Info.MaxPlayerNum	= ZoneInfo[i].MaxPlayerNum;
		Info.ServicePort	= ZoneInfo[i].ServicePort;
		Info.Sequence		= ZoneInfo[i].Sequence;
		m_ZoneInfoList[Info.ZoneId] = Info;
	}
	return true;
}*/

bool CServerManager::readWorldSetting()
{
	TBLWorld tb_world(SERVER_CLASS_NAME::GetActorDB());
	tb_world.QueryWorldSetting(m_WorldSetting);
    return true;
}

void CServerManager::PackGateData(Base::BitStream &Packet, U8 netProvider)
{
	OLD_DO_LOCK(m_cs);

    int size = 0;

    for(HashLineGateMap::iterator iter = m_LineGateMap.begin(); iter != m_LineGateMap.end(); ++iter)
    {
        ++size;
    }
		
	Packet.writeInt( size, 8 );

	HashLineGateMap::iterator itl = m_LineGateMap.begin();

	while(itl != m_LineGateMap.end())
	{
		Packet.writeInt(itl->first,Base::Bit8);
		Packet.writeInt((int)itl->second.size()	,Base::Bit8);

		HashSocketMap::iterator itg = itl->second.begin();

		while(itg!=itl->second.end())
		{
			char IP[32] = "";
			Packet.writeInt(itg->second->GateId	,Base::Bit16);
			//根据网络提供商netProvider获取IP
			if (netProvider >= MAX_IPSET)
			{
				netProvider = 0;
			}

			if (0 != itg->second->Ip[netProvider])
			{
				Packet.writeInt(itg->second->Ip[netProvider],Base::Bit32);
				in_addr ia;
				ia.s_addr = itg->second->Ip[netProvider];
				char* tempIp = inet_ntoa(ia);
				dStrcpy(IP, sizeof(IP), tempIp);
				Packet.writeString(IP, sizeof(IP));
			}
			else
			{
				Packet.writeInt(itg->second->Ip[0],Base::Bit32);
				in_addr ia;
				ia.s_addr = itg->second->Ip[0];
				char* tempIp = inet_ntoa(ia);
				dStrcpy(IP, sizeof(IP), tempIp);
				Packet.writeString(IP, sizeof(IP));
			}

			//---------ylb web-------//
			//Packet.writeInt(itg->second->Port	,Base::Bit16);
			Packet.writeInt(ntohs(itg->second->Port)	,Base::Bit16);
			itg++;
		}

		Packet.writeInt( 0, Base::Bit32 );
		itl++;
	}
}

bool CServerManager::RandGateAddress(int line,int& ip,int& port,U8 netProvider)
{
    OLD_DO_LOCK(m_cs);

    HashLineGateMap::iterator iter = m_LineGateMap.find(line);

    if (iter == m_LineGateMap.end())
        return false;
    
    int size = iter->second.size();

    if (0 == size)
        return false;

    HashSocketMap::iterator itg = iter->second.begin();
    int randGate = rand() % size;
            
    while(itg != iter->second.end() && randGate >= 0)
    {
        ip   = itg->second->Ip[0];
        port = itg->second->Port;

        itg++;
        randGate--;
    }
    
    return true;
}

// 得到所有的GateServer
void CServerManager::GetAllGates( list<stServerInfo*>& serverInfoList )
{
	OLD_DO_LOCK(m_cs);

	HashSocketMap::iterator itg;
	HashLineGateMap::iterator itl = m_LineGateMap.begin();
	while(itl != m_LineGateMap.end())
	{
		itg = itl->second.begin();
		while(itg!=itl->second.end())
		{
			serverInfoList.push_back( itg->second );
			itg++;
		}
		itl++;
	}
}

void CServerManager::GetAllGates( std::list<stServerInfo*>& serverInfoList, int nLineId )
{
	OLD_DO_LOCK(m_cs);

	HashSocketMap::iterator itg;
	HashLineGateMap::iterator itl = m_LineGateMap.find( nLineId );
	if( itl != m_LineGateMap.end())
	{
		itg = itl->second.begin();
		while(itg!=itl->second.end())
		{
			serverInfoList.push_back( itg->second );
			itg++;
		}
	}
}

int CServerManager::GetGateSocket(int playerId)
{
    DO_LOCK(Lock::Player);

    AccountRef spAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if (!spAccount)
        return 0;

    return spAccount->GetGateSocketId();
}

U32 CServerManager::GetNormalLineCount(void)
{
	OLD_DO_LOCK(m_cs);

	return m_LineGateMap.size();
}

U32 CServerManager::GetMaxPlayerCount(void)
{
    return GetNormalLineCount() * g_maxLinePlayers;
}

void CServerManager::GetAllLines(std::vector<int>& lines)
{
	OLD_DO_LOCK(m_cs);

	for (HashLineGateMap::iterator iter = m_LineGateMap.begin(); iter != m_LineGateMap.end(); ++iter)
	{
		lines.push_back(iter->first); 
	}	
}

void CServerManager::Update(void)
{
    OLD_DO_LOCK(m_cs);

    if (m_PacketLogTimer.CheckTimer())
	{
        try
        {
            SERVER->GetServerSocket()->DumpPackets(SERVER->GetPlayerManager()->GetOnlineAccountCount());
            DumpRecvPackets();
            DBStat::Instance()->Dump();
        }
        catch(...)
        {
        }
    }

	if (m_TimeTraceTimer.CheckTimer())
	{
		SERVER->GetRemoteAccountSocket()->CheckTimeEclipse();
		SERVER->GetServerSocket()->CheckTimeEclipse();
		SERVER->GetWorkQueue()->CheckTimeEclipse();
		SERVER->GetSocialWorkQueue()->CheckTimeEclipse();
	}
}

void CServerManager::SetOpenTime(time_t openTime)
{
    OLD_DO_LOCK(m_cs);
    m_openTime = openTime;
}

void CServerManager::AddLoginTimeRange(const TimeRange& range)
{
    OLD_DO_LOCK(m_cs);
    m_loginTimeRange.push_back(range);
}

bool CServerManager::IsOpenTime(time_t curTime)
{
    OLD_DO_LOCK(m_cs);    
    return curTime >= m_openTime;
}

bool CServerManager::IsLoginTime(time_t curTime)
{
    OLD_DO_LOCK(m_cs);

    for (size_t i = 0; i < m_loginTimeRange.size(); ++i)
    {
        if (m_loginTimeRange[i].IsInRange(curTime))
            return true;
    }

    return false;
}

bool CServerManager::HasDataAgent(void) const
{
    return m_DataAgentMap.size() != 0;
}


