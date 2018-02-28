#include "SafePacket.h"
#include "AccountHandler.h"
#include "ServerMgr.h"
#include "PlayerMgr.h"
#include "WinTCP/IPacket.h"
#include "WinTCP/dtServerSocket.h"

SafePacket::SafePacket(void)
{
}

bool SafePacket::Send(AccountRef spPlayer,Base::BitStream& sendPacket)
{
    if (!spPlayer)
        return false;

    stPacketHead* pHead = (stPacketHead*)sendPacket.getBuffer();

    if (pHead->DestServerType != SERVICE_ZONESERVER && pHead->DestServerType != SERVICE_CLIENT)
    {
        assert(0);
        return false;
    }

    if (spPlayer->CanSendLogicData())
    {
        return DoSend(spPlayer.get(),sendPacket,false);
    }
    else
    {
        OLD_DO_LOCK(m_cs);

        //缓冲下消息包
        std::vector<DelayPacket>& pkts = m_packets[spPlayer->GetPlayerId()];
        
        U32 dataSize = sendPacket.getPosition();

        DelayPacket pkt;
        pkt.pData = new char[dataSize];
        pkt.size  = dataSize;

        memcpy(pkt.pData,(char*)pHead,dataSize);

        pkts.push_back(pkt);
    }

    return true;
}

void SafePacket::Flush(Player* pPlayer)
{
    if (0 == pPlayer)
        return;

    OLD_DO_LOCK(m_cs);

    std::vector<DelayPacket>& pkts = m_packets[pPlayer->GetPlayerId()];

    if (0 == pkts.size())
        return;

    for (size_t i = 0; i < pkts.size(); ++i)
    {
        DelayPacket& pkt = pkts[i];

        if (0 == pkt.pData)
            continue;

        Base::BitStream sendPacket(pkt.pData,pkt.size);
        sendPacket.setPosition(pkt.size);

        DoSend(pPlayer,sendPacket,true);

        delete []pkt.pData;
    }

    pkts.clear();
}

bool SafePacket::DoSend(Player* pPlayer,Base::BitStream& sendPacket,bool isBuffed)
{
    if (0 == pPlayer)
        return false;

    stPacketHead* pHead = (stPacketHead*)sendPacket.getBuffer();

    //直接发送玩家数据
    if (pHead->DestServerType == SERVICE_ZONESERVER)
    {
        /*if (isBuffed)
        {
            pHead->DestZoneId = pPlayer->GetLastZoneId();
        }

        //这里还是走查询的方式
        int svrSocket = SERVER->GetServerManager()->GetGateSocket(pPlayer->GetLastLineId(),pPlayer->GetLastZoneId());

        if (svrSocket < 0)
            return false;

        SERVER->GetServerSocket()->Send(svrSocket,sendPacket);*/
    }
    else
    {
        SERVER->GetServerSocket()->Send(pPlayer->GetGateSocketId(),sendPacket);
    }

    return true;
}

void SafePacket::Update(void)
{
    OLD_DO_LOCK(m_cs);

    if (0 == m_packets.size())
        return;

    for (PACKETS_MAP::iterator iter = m_packets.begin(); iter != m_packets.end();)
    {
        U32 playerId = iter->first;
        
        //如果玩家不在线,则删除所有消息包
        if (!SERVER->GetPlayerManager()->IsPlayerOnline(playerId))
        {
            std::vector<DelayPacket>& pkts = iter->second;

            for (size_t i = 0; i < pkts.size(); ++i)
            { 
                DelayPacket& pkt = pkts[i];

                if (0 == pkt.pData)
                    continue;

                delete []pkt.pData;
            }

            iter = m_packets.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}