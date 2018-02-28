#include "stdafx.h"
#include <math.h>

#include "Challenge.h"
#include "PlayerMgr.h"
#include "safepacket.h"
#include "CopymapManager.h"
#include "Arena.h"
#include "Combat.h"
#include <math.h>
#include "ChatHelper.h"
#include "TopManager.h"
#include "wintcp/dtServerSocket.h"

#include "DB_SaveChallenge.h"
#include "DBLayer/data/TBLArena.h"


ChallengeManager::ChallengeManager(void)
{
    m_pCopymapData = 0;
    m_lastRank     = 0;
    m_started      = false;
}

void ChallengeManager::Init()
{
    assert(0 == m_members.size());

    g_Log.WriteLog("正在读取竞技场玩家数据");
    
    std::vector<ChallengeMember*> members;

    TBLArena tblArena(SERVER->GetActorDB());
    DBError err = tblArena.LoadChallengeMembers(members);

    if (DBERR_NONE != err)
    {
        g_Log.WriteError("无法载入玩家竞技场信息");
        return;
    }

    memset(m_rank,0,sizeof(m_rank));

    std::map<U32,U32> rankmap;
   
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (0 == members[i])
            continue;

        m_members[members[i]->playerId] = members[i];

        PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(members[i]->playerId);

        if (spData)
        {
            members[i]->data.level = spData->Level;
        }

        if (members[i]->rank < MAX_BATTLE_RANK)
        {
            rankmap[members[i]->rank] = members[i]->playerId;
        }
    }
        
    int i = 0;

    for (std::map<U32,U32>::iterator iter = rankmap.begin(); iter != rankmap.end(); ++iter,i++)
    {
        m_rank[i] = iter->second;
    }

    m_lastRank = rankmap.size();

    if (m_lastRank > 9999)
    {
        m_lastRank = 9999;
    }
    
    g_Log.WriteLog("读取竞技场玩家数据完毕");

    m_pCopymapData = 0;

    //查找地图
    CCopyMapManager::ID_MAPDATA_MAP& datas = SERVER->GetCopyMapManager()->GetDatas();

    for (CCopyMapManager::ID_MAPDATA_MAP::iterator iter = datas.begin(); iter != datas.end(); ++iter)
    {
        CopymapData* pData = iter->second;

        if (pData->mCopymapType == COPYMAP_TYPE_CHALLENGE)
        {
            m_pCopymapData = pData;
            break;
        }
    }

    if (0 == m_pCopymapData)
    {
        g_Log.WriteLog("找不到竞技场地图");
        return;
    }

    if (m_members.size() > 0)
    {
        m_started = true;
    }
}

bool ChallengeManager::Start(U32 playerId,const ChallengeData& data)
{
    if (0 != m_members.size())
        return true;

    g_Log.WriteLog("竞技场挑战开始了,第1个玩家是[%d]",playerId);
    
    //第一个玩家默认是第1名,后面玩家从无名次开始
    ChallengeMember* pMember = AddMemeber(playerId);
    pMember->rank = 0;
    pMember->data = data;
    InitCount(pMember);

    m_rank[0]  = playerId;
    m_lastRank = 1;

    //保存到数据库
    SaveMember(pMember);

    m_started = true;
    return true;
}

void ChallengeManager::Close()
{
    for (CHALLENGE_MEMBER::iterator iter = m_members.begin(); iter != m_members.end(); ++iter)
    {
        ChallengeMember* pMember = iter->second;

        if (0 == pMember)
            continue;

        SaveMember(pMember);
    }
}

ChallengeMember* ChallengeManager::UpdateMemberData(U32 playerId,const ChallengeData& data,bool createIfNotExist)
{
    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
    {
        if (!createIfNotExist)
            return 0;

        pMember = AddMemeber(playerId);
        InitCount(pMember);
        pMember->data = data;

        SaveMember(pMember);
    }

    pMember->data = data;
    return pMember;
}

bool ChallengeManager::AddOnlineChallenge(U32 playerId,U32 targetId)
{
    OnlineChallenge* pArena = new OnlineChallenge(SERVER->GetArenaManager()->GenArenaId());

    if (0 == pArena)
        return false;

    pArena->m_player1 = playerId;
    pArena->m_player2 = targetId;
    pArena->m_sTime   = time(0) + 11;
    
    m_arenas[pArena->GetId()] = pArena;

    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if(spPlayer)
    {
        spPlayer->m_arenaId = pArena->GetId();
    }

    AccountRef spTarget = SERVER->GetPlayerManager()->GetOnlinePlayer(targetId);

    if(spTarget)
    {
        spTarget->m_arenaId = pArena->GetId();
    }

    return true;
}


bool ChallengeManager::AddOfflineChallenge(U32 playerId,U32 targetId)
{
    OnlineChallenge* pArena = new OnlineChallenge(SERVER->GetArenaManager()->GenArenaId());

    if (0 == pArena)
        return false;

    pArena->m_player1 = playerId;
    pArena->m_player2 = targetId;
    pArena->m_sTime   = time(0) + 5;
    
    m_arenas[pArena->GetId()] = pArena;
    return true;
}

void ChallengeManager::AcceptChallenge(U32 playerId)
{
    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if(!spPlayer)
    {
        return;
    }

    OnlineChallenge* pArena = FindArena(spPlayer->m_arenaId);

    if (0 == pArena)
        return;

    //判断是否可以开始
    if (0 == pArena->m_sTime)
        return;

    if (pArena->m_player1 != playerId && pArena->m_player2 != playerId)
        return;

    //开始竞技场
    if (StartOnlineChallenge(pArena))
    {
        pArena->m_ack = true;
    }
}

void ChallengeManager::MarkOpen(U32 playerId,bool isOpened)
{
    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
        return;

    pMember->active = isOpened;
}

bool ChallengeManager::SendBaseInfo(U32 playerId)
{
    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if(!spPlayer)
    {
        return false;
    }

    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
        return false;

    char buf[1024]; 
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead *pSendHead = IPacket::BuildPacketHead(sendPacket,WC_CHALLENGE_BASEINFO,spPlayer->AccountId,SERVICE_CLIENT);

    sendPacket.writeInt(pMember->rank + 1,32);
    sendPacket.writeInt(pMember->wins,32);
    sendPacket.writeInt(pMember->total,32);
    sendPacket.writeInt(pMember->arenaPoints,32);
    sendPacket.writeInt(pMember->nChallengeBy,32);
    sendPacket.writeInt(pMember->nLosed,32);
    sendPacket.writeInt(pMember->onlineLimit,32);
    sendPacket.writeInt(pMember->offlineLimit,32);
    sendPacket.writeInt(pMember->extraLimit,32);
    pMember->data.Pack(&sendPacket);

    //战报信息
    int count = pMember->reports.size();
    sendPacket.writeInt(count,32);

    for (std::list<BattleReport>::iterator iter = pMember->reports.begin(); iter != pMember->reports.end(); ++iter)
    {
        BattleReport& report = (*iter);

        sendPacket.writeFlag(report.self);
        sendPacket.writeInt(report.time,32);
        sendPacket.writeInt(report.targetId,32);
        sendPacket.writeString(report.targetName);
        sendPacket.writeInt(report.oldRank + 1,32);
        sendPacket.writeInt(report.newRank + 1,32);
    }

    pSendHead->PacketSize = sendPacket.getPosition()-IPacket::GetHeadSize();
	SERVER->GetServerSocket()->Send(spPlayer->GetGateSocketId(),sendPacket);
    
    return true;
}

bool ChallengeManager::SendTopMembers(U32 playerId)
{
    int count = min(3,m_lastRank);

    if (0 == count)
        return false;
    
    char buf[1024]; 
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead *pSendHead = IPacket::BuildPacketHead(sendPacket,WC_CHALLENGE_TOP3,0,SERVICE_CLIENT);
    pSendHead->DestZoneId = 0;

    for (int i = 0; i < count; ++i)
    {
        ChallengeMember* pMember = GetMember(m_rank[i]);

        if (0 == pMember)
        {
            continue;
        }

        PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(pMember->playerId);

        if (!spData)
        {
            continue;
        }

        sendPacket.writeInt(pMember->playerId,32);
        sendPacket.writeString(spData->PlayerName,32);

        pSendHead->DestZoneId++;
    }

    pSendHead->PacketSize = sendPacket.getPosition()-IPacket::GetHeadSize();

    if (0 == playerId)
    {
        Broadcast(sendPacket);
    }
    else
    {
        AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

        if(!spPlayer)
        {
            return false;
        }
        
        pSendHead->Id = spPlayer->AccountId;
        SERVER->GetServerSocket()->Send(spPlayer->GetGateSocketId(),sendPacket);
    }

    return true;
}

void ChallengeManager::SelectByRank(U32 curRank,std::list<U32>& idList)
{
    //第1名无需比赛
    if (0 == curRank)
        return;

    if (idList.size() >= 5)
        return;

    //从当前实际排名的玩家数开始查找
    if (curRank > m_lastRank)
    {
        curRank = m_lastRank;
    }

    if (curRank >= 9999)
    {
        int randpos = (rand() % 9) + 1;

        for (int i = 0; i < 5; ++i)
        {
            int rank = 999 * 10 + randpos - i * 100;
            idList.push_back(rank);
        }

        return;
    }

    if (curRank > 2520 && curRank < 9999)
    {
        curRank = curRank - 100;
        idList.push_back(curRank);

        return SelectByRank(curRank,idList);
    }

    if (curRank > 1320 && curRank <= 2520)
    {
        curRank = curRank - 10;
        idList.push_back(curRank);

        return SelectByRank(curRank,idList);
    }

    if (curRank > 720 && curRank <= 1320)
    {
        curRank = curRank - 5;
        idList.push_back(curRank);

        return SelectByRank(curRank,idList);
    }

    if (curRank > 360 && curRank <= 720)
    {
        curRank = curRank - 3;
        idList.push_back(curRank);

        return SelectByRank(curRank,idList);
    }

    if (curRank > 120 && curRank <= 360)
    {
        curRank = curRank - 2;
        idList.push_back(curRank);

        return SelectByRank(curRank,idList);
    }

    if (curRank > 0 && curRank <= 120)
    {
        curRank = curRank - 1;
       
        if (curRank >= 0)
        {
            idList.push_back(curRank);
            return SelectByRank(curRank,idList);
        }
    }
}

bool ChallengeManager::SendChallengers(U32 playerId)
{
    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if(!spPlayer)
    {
        return false;
    }

    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
    {
        pMember = AddMemeber(playerId);
    }

    if (0 == pMember)
        return false;

    std::list<U32> idList;
    SelectByRank(pMember->rank,idList);

    //如果人数不满5人，向后补充人数
    if (idList.size() < 5)
    {   
        int nextRank = pMember->rank + 1;

        while(nextRank < m_lastRank && idList.size() < 5)
        {
            idList.push_front(nextRank);
            nextRank++;
        }
    }

    char buf[1024]; 
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WC_CHALLENGE_TARGETS,spPlayer->AccountId,SERVICE_CLIENT);

    int count = 0;

    for (std::list<U32>::iterator iter = idList.begin(); iter != idList.end(); ++iter)
    {
        ChallengeMember* pMember = GetMember(m_rank[*iter]);

        if (0 == pMember)
            continue;

        if (!PackMemberInfo(pMember,sendPacket))
            continue;

        ////把玩家加入通知队列
        //AccountRef spMember = SERVER->GetPlayerManager()->GetOnlinePlayer(pMember->playerId);

        //if(spMember)
        //{
        //    spMember->m_challengeNotifyList[playerId] = true;
        //}

        count++;
    }

    pSendHead->DestZoneId = count;
    pSendHead->PacketSize = sendPacket.getPosition()-IPacket::GetHeadSize();
    SERVER->GetServerSocket()->Send(spPlayer->GetGateSocketId(),sendPacket);

    return true;
}

ChallengeMember* ChallengeManager::GetMember(U32 playerId)
{
    CHALLENGE_MEMBER::iterator iter = m_members.find(playerId);

    if (iter == m_members.end())
        return 0;

    return iter->second;
}

ChallengeMember* ChallengeManager::AddMemeber(U32 playerId)
{
    CHALLENGE_MEMBER::iterator iter = m_members.find(playerId);

    if (iter != m_members.end())
        return 0;

    ChallengeMember* pMember = new ChallengeMember;
    pMember->playerId   = playerId;
    m_members[playerId] = pMember;

    return pMember;
}

bool ChallengeManager::InitCount(ChallengeMember* pMember)
{
    if (0 == pMember)
        return false;

    bool isChanged = (pMember->nChallengeBy != 0 ||
                      pMember->nLosed != 0 ||
                      pMember->onlineLimit != 8 ||
                      pMember->offlineLimit != 4);
    
    pMember->nChallengeBy = 0;
    pMember->nLosed       = 0;
    pMember->onlineLimit  = 8;
    pMember->offlineLimit = 4;

    return isChanged;
}

bool ChallengeManager::SendJoinRequest(U32 myId,const char* myName,U32 targetId)
{
    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(targetId);

    if(!spPlayer)
    {
        return false;
    }

    char buf[256];
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WZ_CHALLENGE_INVITE,targetId,SERVICE_ZONESERVER,spPlayer->GetLastZoneId());
    sendPacket.writeInt(myId,32);
    sendPacket.writeString(myName);
    
    pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
    SAFEPKT->Send(spPlayer,sendPacket);

    return true;
}

bool ChallengeManager::SendChallengeStart(U32 myId,U32 targetId,bool isOfflineMode)
{
    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(myId);

    if(!spPlayer)
    {
        return false;
    }

    char buf[64];
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WC_CHALLENGE_START,spPlayer->AccountId,SERVICE_CLIENT);
    sendPacket.writeInt(targetId,32);
    sendPacket.writeFlag(isOfflineMode);
    
    pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
    SERVER->GetServerSocket()->Send(spPlayer->GetGateSocketId(),sendPacket);
    return true;
}

bool ChallengeManager::StartOnlineChallenge(OnlineChallenge* pArena)
{
    if (0 == m_pCopymapData)
    {
        g_Log.WriteLog("挑战地图不存在，无法在线挑战");
        return false;
    }
    
    //创建副本
    int copymapId = 1;
    int nErr = SERVER->GetCopyMapManager()->CreateCopymap(pArena->m_player1,m_pCopymapData->mId,copymapId);

    if (1 == copymapId || COPYMAP_ERROR_NONE != nErr)
    {
        g_Log.WriteError("玩家[%d]无法开启副本[%d],无法挑战",pArena->m_player1,m_pCopymapData->mId);
        return false;
    }

    pArena->m_copymapId = copymapId;

    CopymapInst* pCopymap = SERVER->GetCopyMapManager()->GetCopymap(copymapId);

    if (0 == pCopymap)
    {
        return false;
    }
    
    pCopymap->SetArenaId(pArena->GetId());

    //传送玩家进入副本
    if (!SendEnterChallenge(pArena->m_player1,m_pCopymapData->mId,copymapId,1) || !SendEnterChallenge(pArena->m_player2,m_pCopymapData->mId,copymapId,2))
        return false;
    
    return true;
}

bool ChallengeManager::StartOfflineChallenge(U32 playerId,U32 targetId)
{
    ChallengeMember* p1 = GetMember(playerId);
    ChallengeMember* p2 = GetMember(targetId);

    if (0 != p1 && 0 != p2)
    {
        return StartOfflineChallenge(p1,p2);
    }

    return false;
}

bool ChallengeManager::StartOfflineChallenge(ChallengeMember* p1,ChallengeMember* p2)
{
    if (0 == p1 || 0 == p2)
    {
        return false;
    }

    p1->starter = true;

    //计算玩家的比赛结果值
    //离线计算公式＝(自身品质-对方品质)/10×0.5+(自身竞技熟练度-对方竞技熟练度)×0.25+
    //((自身技能总熟练度+天赋点数+角色等级)-(对方技能总熟练度+天赋点数+角色等级))×1+
    //(自身灵兽换算积分-对手灵兽换算积分)/10×0.1

    float rslt = 0.5f * (p1->data.quality - p2->data.quality) / 10.0f  + min(800,(p1->arenaPoints - p2->arenaPoints)) * 0.25f + 
        ((p1->data.nSkills + p1->data.nTalents + p1->data.level) - (p2->data.nSkills + p2->data.nTalents + p2->data.level)) * 1.0f + 
        0.1f * (p1->data.nPetSkills - p2->data.nPetSkills) / 10.0f;
    
    bool isWin = rslt > 0;

    if (0 == rslt)
    {   
        if (p1->data.quality != p2->data.quality)
        {
            isWin = p1->data.quality > p2->data.quality;
        }
        else
        {
            isWin = p1->arenaPoints >= p2->arenaPoints;
        }
    }
    
    if (isWin)
    {
        UpdateStat(p1,p2);
    }
    else
    {
        UpdateStat(p2,p1);
    }

    //向玩家发送竞技相关信息
    SendChallengeResult(p1,p2->playerId,p2->arenaPoints,isWin);

    return true;
}

bool ChallengeManager::SendEnterChallenge(U32 playerId,U32 mapId,U32 copymapId,U32 force)
{
    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if(!spPlayer)
    {
        return false;
    }

    //退出玩家所在的战场排队
    SERVER->GetCombatManager()->Leave(playerId,spPlayer->m_combatId);

    char buf[128];
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WZ_ARENA_ENTER,playerId,SERVICE_ZONESERVER,spPlayer->GetLastZoneId());
    sendPacket.writeInt(mapId,Base::Bit32);
    sendPacket.writeInt(copymapId,Base::Bit32);
    sendPacket.writeInt(force,Base::Bit32);
    pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );

    return SAFEPKT->Send(spPlayer,sendPacket);
}

void ChallengeManager::IncOnlineCount(U32 playerId,int count)
{
    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
        return;

    pMember->onlineLimit += count;
}

void ChallengeManager::IncOfflineCount(U32 playerId,int count)
{
    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
        return;

    pMember->offlineLimit += count;
}

void ChallengeManager::IncExtraCount(U32 playerId,int count)
{
    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
        return;

    pMember->extraLimit += count;

    g_Log.WriteLog("玩家[%d]购买了额外战斗次数",playerId);

    //刷新客户端基本信息
    SendBaseInfo(playerId);

    //提示客户端购买成功
    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if(!spPlayer)
    {
        return;
    }

    ClientErrorCode errorCode(30321);
    spPlayer->SendClientErrorCode(errorCode.GetStream());

    SaveMember(pMember);
}

void ChallengeManager::ClearBattleReport(U32 playerId)
{
    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
        return;

    pMember->reports.clear();
}

bool ChallengeManager::IsInRank(U32 playerId)
{
    ChallengeMember* pMember = GetMember(playerId);

    if (0 == pMember)
        return false;

    return pMember->rank < 9999;
}

bool ChallengeManager::CanChallenge(ChallengeMember* pMember,U32 targetId)
{
    if (0 == pMember)
        return false; 

    bool isOnlineTarget = SERVER->GetPlayerManager()->IsPlayerOnline(targetId);

    if (0 == pMember->onlineLimit && 0 == pMember->offlineLimit && 0 == pMember->extraLimit)
    {
        return false;
    }

    if (isOnlineTarget)
    {
        return pMember->onlineLimit || pMember->extraLimit;
    }
    else
    {
        return pMember->offlineLimit || (pMember->onlineLimit - 4 > 0) || pMember->extraLimit;
    }
}

bool ChallengeManager::PackMemberInfo(ChallengeMember* pMember,Base::BitStream& sendPacket)
{
    if (0 == pMember)
        return false;

    PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(pMember->playerId);

    if (!spData)
    {
        //这里也返回数据好做测试！
        sendPacket.writeInt(pMember->playerId,32);
        sendPacket.writeString("测试角色",32);
        sendPacket.writeInt(0,32);
        sendPacket.writeInt(0,8);
        sendPacket.writeInt(0,8);
        sendPacket.writeInt(pMember->rank + 1,32);
        sendPacket.writeInt(0,32);
        sendPacket.writeInt(0,32);
        sendPacket.writeInt(pMember->arenaPoints,32);
        sendPacket.writeFlag(SERVER->GetPlayerManager()->IsPlayerOnline(pMember->playerId));
        pMember->data.Pack(&sendPacket);

        return true;
    }
    
    sendPacket.writeInt(pMember->playerId,32);
    sendPacket.writeString(spData->PlayerName,32);
    sendPacket.writeInt(spData->FaceImage,32);
    sendPacket.writeInt(spData->Family,8);
    sendPacket.writeInt(spData->Sex,8);
    sendPacket.writeInt(pMember->rank + 1,32);
    sendPacket.writeInt(spData->Level,32);
    sendPacket.writeInt(spData->FirstClass,32);
    sendPacket.writeInt(pMember->arenaPoints,32);
    sendPacket.writeFlag(SERVER->GetPlayerManager()->IsPlayerOnline(pMember->playerId));
    pMember->data.Pack(&sendPacket);
    
    return true;
}

void ChallengeManager::SaveMember(ChallengeMember* pMember)
{
    if (0 == pMember)
        return;

    if (pMember->rank >= 9999)
        return;
    
    DB_SaveChallenge* pDBHandle = new DB_SaveChallenge;
    pDBHandle->m_member = *pMember;
    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}

void ChallengeManager::OnNewDay(void)
{
    //新的一天重置玩家的战斗计数等信息
    for (CHALLENGE_MEMBER::iterator iter = m_members.begin(); iter != m_members.end(); ++iter)
    {
        ChallengeMember* pMember = iter->second;

        if (0 == pMember)
            continue;

        if (InitCount(pMember))
        {
            SaveMember(pMember);

            if (pMember->active)
            {
                //刷新客户端显示
                SendBaseInfo(pMember->playerId);
            }
        }
    }
}

void ChallengeManager::DecChallengeCount(ChallengeMember* pMember)
{
    if (0 == pMember)
        return;

    //扣减玩家的挑战计数
    if (pMember->offlineMode)
    {
        if (pMember->offlineLimit > 0)
        {
            pMember->offlineLimit--;
        }
        else
        {
            if (pMember->onlineLimit > 4)
            {
                pMember->onlineLimit -= 4;
            }
            else
            {
                if (pMember->extraLimit > 0)
                {
                    pMember->extraLimit--;
                }
            }
        }
    }
    else
    {
        if (pMember->onlineLimit > 0)
        {
            pMember->onlineLimit--;
        }
        else
        {
            if (pMember->extraLimit > 0)
            {
                pMember->extraLimit--;
            }
        }
    }
}

void ChallengeManager::UpdateStat(U32 playerId1,U32 playerId2,bool isManual)
{
    ChallengeMember* p1 = GetMember(playerId1);
    ChallengeMember* p2 = GetMember(playerId2);

    if (0 != p1 && 0 != p2)
    {
        UpdateStat(p1,p2,isManual);
    }
}

void ChallengeManager::UpdateStat(ChallengeMember* p1,ChallengeMember* p2,bool isManual)
{
    if (0 == p1 || 0 == p2)
    {
        return;
    }

    if (isManual)
    {
        //修改双方的竞技场熟练度

        //胜利方本场获得竞技熟练度＝10×（对手品质×2＋对手竞技熟练度×10)/（自身品质＋对手品质＋自身竞技熟练度×10），最高获得20点
        //失败方本场获得竞技熟练度＝4×（对手品质×1.5＋对手竞技熟练度×10)/（自身品质＋对手品质＋自身竞技熟练度×10），最低获得2点

        float winPoints  = 10.0f * (p2->data.quality * 2.0f + p2->arenaPoints * 10.0f) / (p1->data.quality + p2->data.quality + p1->arenaPoints * 10.0f) + 0.5f;
        float losePoints = 4.0f  * (p2->data.quality * 1.5f + p2->arenaPoints * 10.0f) / (p1->data.quality + p2->data.quality + p1->arenaPoints * 10.0f) + 0.5f;

        winPoints  = min(20.0f,winPoints);
        losePoints = max(2.0f,losePoints);

        p1->arenaPoints += (int)winPoints;
        p1->curPoints   =  (int)winPoints;
        p2->arenaPoints += (int)losePoints;
        p2->curPoints   =  (int)losePoints;

        g_Log.WriteLog("角色[%d]胜利获得竞技场熟练度[%d]",p1->playerId,winPoints);
        g_Log.WriteLog("角色[%d]失败获得竞技场熟练度[%d]",p2->playerId,losePoints);
    }
    
    //交换排名
    bool isRankChanged = false;

    U32 oldRank1 = p1->rank;
    U32 oldRank2 = p2->rank;

    if (p2->rank < p1->rank && !p2->starter)
    {
        std::swap(p1->rank,p2->rank);
        isRankChanged = true;
    }

    if (p2->rank >= MAX_BATTLE_RANK && m_lastRank < MAX_BATTLE_RANK)
    {
        p2->rank = m_lastRank;
        m_lastRank++;

        if (m_lastRank > 9999)
        {
            m_lastRank = 9999;
        }

        isRankChanged = true;
    }

    m_rank[p1->rank] = p1->playerId;
    m_rank[p2->rank] = p2->playerId;

    //计数
    p1->wins++;
    p2->wins = 0;
    p1->total++;
    p2->total++;

    //递减玩家的挑战次数
    if (p1->starter)
    {
        DecChallengeCount(p1);
    }

    if (p2->starter)
    {
        DecChallengeCount(p2);
    }
    
    //记录玩家的被挑战数和失败数
    if (p1->starter)
    {
        p2->nChallengeBy++;
        p2->nLosed++;
    }
    else
    {
        p1->nChallengeBy++;
    }

    //增加失败玩家的战报信息
    BattleReport r2;
    r2.self     = p1->starter ? false : true;
    r2.targetId = p1->playerId;

    PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(r2.targetId);

    if (spData)
    {
        dStrcpy(r2.targetName,sizeof(r2.targetName),spData->PlayerName);
    }
    
    r2.time     = _time32(0);
    r2.oldRank  = oldRank2;
    r2.newRank  = p2->rank;

    p2->reports.push_back(r2);

    if (p2->reports.size() > MAX_BATTLE_REPORT)
    {
        p2->reports.pop_front();
    }

    //如果玩家连胜20场,则广播玩家数据
    if (p1->wins >= 20)
    {
        SendWinsPlayer(p1);   
    }

    //如果玩家新入100名。发送新星公告
    if (oldRank1 >= 100)
    {
        if (p1->rank < 100)
            SendNewstar(p1);
    }
    else
    {
        if (p1->rank < 3 && p1->rank != oldRank1)
        {
            //发布公告有玩家进入3甲
            SendNewTop(p1);
        }
    }

    //更新玩家新的称号
    if (p1->rank != oldRank1)
    {
        SendRankUpdated(p1);
    }

    if (p2->rank != oldRank2)
    {
        SendRankUpdated(p2);
    }

    if (isRankChanged && (p1->rank < 3 || p2->rank < 3))
    {
        SendTopMembers(0);
    }

    //发送最新的玩家数据
    SendBaseInfo(p1->playerId);
    SendBaseInfo(p2->playerId);

    //保存玩家数据到DB
    SaveMember(p1);
    SaveMember(p2);

    if (oldRank1 != p1->rank && p1->active)
    {
        SendChallengers(p1->playerId);
    }

    if (oldRank2 != p2->rank && p2->active)
    {
        SendChallengers(p2->playerId);
    }

    p1->starter = false;
    p2->starter = false;

    //更新排行榜数据
    SERVER->GetTopManager()->onArenaDataCommingEX(eTopType_ArenaPointNew,p1->playerId,p1->rank + 1,p1->data.quality);
    SERVER->GetTopManager()->onArenaDataCommingEX(eTopType_ArenaPointNew,p2->playerId,p2->rank + 1,p2->data.quality);
    SERVER->GetTopManager()->UpdateTop(eTopType_ArenaPointNew);
}

int ChallengeManager::GetLastPoints(U32 player)
{
    ChallengeMember* pMember = GetMember(player);

    if (0 == pMember)
        return 0;

    return pMember->curPoints;
}

void ChallengeManager::SendWinsPlayer(ChallengeMember* pMember)
{
    if (0 == pMember)
        return;

    PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(pMember->playerId);

    if (!spData)
    {
        return;
    }

    stChatMessage msg;
    msg.btMessageType = CHAT_MSG_TYPE_CHALLENGE;
    msg.nSender       = pMember->playerId;
    dStrcpy(msg.szSenderName,sizeof(msg.szSenderName),spData->PlayerName);
    msg.nRecver       = pMember->wins;
    msg.nSenderLineID = 2;

    CHAT_HELPER->SendMessageToWorld(msg);
}

void ChallengeManager::SendNewstar(ChallengeMember* pMember)
{
    if (0 == pMember)
        return;

    PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(pMember->playerId);

    if (!spData)
    {
        return;
    }

    stChatMessage msg;
    msg.btMessageType = CHAT_MSG_TYPE_CHALLENGE;
    msg.nSender       = pMember->playerId;
    dStrcpy(msg.szSenderName,sizeof(msg.szSenderName),spData->PlayerName);
    msg.nRecver       = pMember->rank + 1;
    msg.nSenderLineID = 1;

    CHAT_HELPER->SendMessageToWorld(msg);
}

void ChallengeManager::SendNewTop(ChallengeMember* pMember)
{
    if (0 == pMember)
        return;

    PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(pMember->playerId);

    if (!spData)
    {
        return;
    }

    stChatMessage msg;
    msg.btMessageType = CHAT_MSG_TYPE_CHALLENGE;
    msg.nSender       = pMember->playerId;
    dStrcpy(msg.szSenderName,sizeof(msg.szSenderName),spData->PlayerName);
    msg.nRecver       = pMember->rank + 1;
    msg.nSenderLineID = 0;

    CHAT_HELPER->SendMessageToWorld(msg);
}

void ChallengeManager::SendRankUpdated(ChallengeMember* pMember)
{
    if (0 == pMember)
        return;

    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(pMember->playerId);

    if(!spPlayer)
    {
        return;
    }

    PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(pMember->playerId);

    if (!spData)
    {
        return;
    }

    char buf[256];
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WZ_CHALLENGE_RANKCHANGED,pMember->playerId,SERVICE_ZONESERVER,spPlayer->GetLastZoneId());
    sendPacket.writeInt(pMember->rank,Base::Bit32);
    pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

    SAFEPKT->Send(spPlayer,sendPacket);
}

void ChallengeManager::Broadcast(Base::BitStream& sendPacket)
{
    stPacketHead* pSendHead = (stPacketHead*)sendPacket.getBuffer();

    for (CHALLENGE_MEMBER::iterator iter = m_members.begin(); iter != m_members.end(); ++iter)
    {
        ChallengeMember* pMember = iter->second;

        if (0 == pMember || !pMember->active)
            continue;

        AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(pMember->playerId);

        if(!spPlayer)
        {
            continue;
        }
        
        pSendHead->Id = spPlayer->GetAccountId();
        SERVER->GetServerSocket()->Send(spPlayer->GetGateSocketId(),sendPacket);
    }
}

OnlineChallenge* ChallengeManager::FindArena(U32 arenaId)
{
    if (0 == arenaId)
        return 0;

    ARENA_MAP::iterator iter = m_arenas.find(arenaId);

    if (iter == m_arenas.end())
    {
        return 0;
    }

    return iter->second;
}

void ChallengeManager::CloseArena(U32 arenaId)
{
    OnlineChallenge* pArena = FindArena(arenaId);

    if (0 == pArena)
        return;

    m_arenas.erase(arenaId);
    delete pArena;
}

void ChallengeManager::Update(void)
{
    time_t curTime = time(0);

    for (ARENA_MAP::iterator iter = m_arenas.begin(); iter != m_arenas.end();)
    {
        OnlineChallenge* pArena = iter->second;

        if (0 == pArena)
        {
            iter = m_arenas.erase(iter);
            continue;
        }

        //1小时候删除竞技场数据
        if (curTime - pArena->m_cTime > 3600 || pArena->m_toDelete)
        {
            delete pArena;
            iter = m_arenas.erase(iter);
        }
        else
        {
            if (pArena->m_online)
            {
                //如果10秒后战斗没确认开始，自动进入离线挑战
                if (0 != pArena->m_sTime && curTime > pArena->m_sTime && (0 == pArena->m_ack))
                {
                    pArena->m_sTime = 0;

                    //提示玩家进入离线竞技模式
                    ChallengeMember* p1 = GetMember(pArena->m_player1);
                    ChallengeMember* p2 = GetMember(pArena->m_player2);

                    if (0 != p1 && 0 != p2)
                    {
                        StartOfflineChallenge(p1,p2);
                    }

                    pArena->m_toDelete = true;
                }
            }
            else
            {
                if (0 != pArena->m_sTime && curTime > pArena->m_sTime)
                {
                    pArena->m_sTime = 0;

                    ChallengeMember* p1 = GetMember(pArena->m_player1);
                    ChallengeMember* p2 = GetMember(pArena->m_player2);

                    if (0 != p1 && 0 != p2)
                    {
                        StartOfflineChallenge(p1,p2);
                    }

                    pArena->m_toDelete = true;
                }
            }

            ++iter;
        }
    }
} 

void ChallengeManager::SendChallengeResult(ChallengeMember* pMember,U32 targetId,U32 arenaPoints,bool wined)
{
    if (0 == pMember)
        return;

    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(pMember->playerId);

    if(!spPlayer)
    {
        return;
    }

    PlayerSimpleDataRef spData = SERVER->GetPlayerManager()->GetPlayerData(pMember->playerId);

    if (!spData)
    {
        return;
    }

    char buf[256];
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WZ_CHALLENGE_OFFLINERSLT,pMember->playerId,SERVICE_ZONESERVER,spPlayer->GetLastZoneId());
    sendPacket.writeFlag(wined);
    sendPacket.writeInt(targetId,32);
    sendPacket.writeInt(arenaPoints,32);
    pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

    SAFEPKT->Send(spPlayer,sendPacket);
}

void ChallengeManager::SendReward(ChallengeMember* pMember)
{
    if (0 == pMember)
        return;

    AccountRef spPlayer = SERVER->GetPlayerManager()->GetOnlinePlayer(pMember->playerId);

    if(!spPlayer)
    {
        return;
    }
    
    char buf[256]; 
    Base::BitStream sendPacket(buf,sizeof(buf));
    stPacketHead *pSendHead = IPacket::BuildPacketHead(sendPacket,WC_CHALLENGE_REWARD,spPlayer->AccountId,SERVICE_CLIENT);

    sendPacket.writeInt(0,32); 
    sendPacket.writeInt(0,32);
    sendPacket.writeInt(0,32);
    pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

    SERVER->GetServerSocket()->Send(spPlayer->GetGateSocketId(),sendPacket);
}

bool ChallengeManager::TimeProcess(bool bExit)
{
    SERVER->GetWorkQueue()->PostEvent(3,0,0,0,WQ_TIMER);
    return true;
}