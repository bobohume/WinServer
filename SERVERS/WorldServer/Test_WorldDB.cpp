#include "stdafx.h"
#include "DBLib/dbLib.h"
#include "zlib/zlib.h"
#include "Common/MemGuard.h"
#include "base/WorkQueue.h"
#include "Common/PacketType.h"
#include "WinTCP/IPacket.h"
#include "WinTCP/dtServerSocket.h"
#include "TeamManager.h"
#include "WorldServer.h"
#include "EventProcess.h"
#include "PlayerMgr.h"
#include "ServerMgr.h"
#include "SavePlayerJob.h"
#include "MailManager.h"
#include "Common/TeamBase.h"
#include "CopymapManager.h"
#include "GM/GMRecver.h"
#include "BanList.h"
#include "SocialManager.h"
#include "EventManager.h"
#include "TopManager.h"

#include "LockCommon.h"
#include "Base/Log.h"
#include "Common/DumpHelper.h"
#include "AccountHandler.h"
#include "ChatServer/PlayerMgr.h"
#include "ChatHelper.h"
#include "DBLayer/Data/TBLAccount.h"
#include "DBLayer/Data/TBLWorld.h"
#include "DBLayer/Data/TBLMailList.h"
#include "Common/ZoneSetting.h"
#include "DBLayer\Data\TBLZoneSetting.h"
#include "QuickLz/quicklz.h"
#include "Org/OrgHandle.h"
#include "ZoneEventManager.h"
#include "CollectMgr.h"
#include "CorpsManager.h"
#include "BillingManager.h"
#include "Common/CommonClient.h"
#include "LogicEventMgr.h"
#include "StockManager.h"
#include "BoardManager.h"
#include "GoldExchangeManager.h"
#include "DB_SendActionKey.h"
#include "DB_SaveActionKey.h"

#include "Org/Org.h"
#include "Org/OrgManager.h"

#include "Common/YYDef.h"
#include "RandTeam.h"
#include "DBLayer/Data/TBLPlayer.h"
#include "DBContext.h"
#include "CommLib/PacketFactoryManager.h"
#include "GamePlayLog.h"
#include "Arena.h"
#include "Combat.h"
#include "MultiMountGroup.h"
#include "DB_ArenaQuery.h"
#include "DB_QueryIp.h"
#include "Common/BonusBase.h"
#include "Org/LeagueManager.h"
#include "Common/RALibDef.h"
#include "Common/ClientErrorCode.h"
#include "common/cfgbuilder.h"
#include "Common/BanInfoData.h"
#include "Common/CollectItemData.h"
#include "Common/ItemData.h"
#include "Common/OrgDepotData.h"
#include "WorldDB_LoadMode_Def.h"
#include <algorithm>
#include "WorldDBX_GetDiffHelper.h"

#include "DB_SetAccountStatus.h"
#include "DB_CheckOldPlayerLogin.h"
#include "DB_SetTopDisaster.h"
#include "DB_SetTopLevel.h"
#include "DB_CheckPlayerLoginBonus.h"
#include "DB_QueryFamilyPos.h"
#include "DB_QueryPlayerByName.h"
#include "BFCorpsChatHelp.h"

#include "GlobalEventManager.h"

//----------------------------------------------------------------------------

#ifdef TEST_WORLDDBX

bool g_isTestWorldDBX = true;

typedef bool (*NetEventFunction)(int ,stPacketHead *,Base::BitStream *);
extern NetEventFunction NetFNList[END_NET_MESSAGE];

//---------------------------------------------------------------------------

bool g_waitchecked = false;
bool g_waitDataLoading = true;
U32  g_curLoading  = 0;
stdext::hash_map<U32,stPlayerStruct> g_testDatas;
stdext::hash_map<U32,U32> g_versionMap;
stdext::hash_map<U32,U32> g_checkCount;
stdext::hash_map<U32,U32> g_timerMap;
std::vector<std::pair<U32,U32> > samplers;
std::vector<stPlayerStruct> datas;
stdext::hash_map<U32,U32> g_checkStatus;

enum CheckStatus
{
    E_CHECK_NONE = 0,
    E_CHECK_SAVEDATA,
    E_CHECK_WAIT,
    E_CHECK_CHECKDATA,

    E_CHECK_COUNT
};



#define RETEST SERVER->GetWorkQueue()->PostEvent(0,(void*)"TestWorldDBX",strlen("TestWorldDBX") + 1,true,WQ_COMMAND );
#define RAND_D(x,y) ((rand() % (y - x)) + x)

void __ChangePlayerData(stPlayerStruct& tPlayer)
{
    U32 playerId = tPlayer.BaseData.PlayerId;
    tPlayer.BaseData.SaveUID  = ++g_versionMap[playerId];

    //change base data
    tPlayer.BaseData.LimitOnlineTime = RAND_D(0,1000);
    tPlayer.BaseData.LastAccountLoginTime = RAND_D(0,1000);
    tPlayer.BaseData.LastLoginTime = RAND_D(0,1000);
    tPlayer.BaseData.LastLogoutTime = RAND_D(0,1000);
    tPlayer.BaseData.TotalOnlineTime = RAND_D(0,1000);
    tPlayer.BaseData.CurrentHP = RAND_D(0,1000);
    tPlayer.BaseData.CurrentMP = RAND_D(0,1000);
    tPlayer.BaseData.CurrentPP = RAND_D(0,1000);

    /*char* pTestBuf = (char*)&tPlayer.DispData;

    for (U32 i = 0; i < sizeof(tPlayer.DispData); ++i)
    {
        pTestBuf[i] = rand() % 5;
    }

    pTestBuf = (char*)&tPlayer.MainData;

    for (U32 i = 0; i < sizeof(tPlayer.MainData); ++i)
    {
        pTestBuf[i] = rand() % 5;
    }*/
}

class CCompressPlayerBuf
{
public:
	char buf[MAX_PLAYER_STRUCT_SIZE];
	U32  size;
};

CCompressPlayerBuf g_compressBuf;
int g_worldDBSocket=0;


bool HandleWWLoadPlayerRespondFromWorldDBX(int SocketHandle, stPacketHead* pHead, Base::BitStream* Packet)
{
    DO_LOCK(Lock::Player);

    U32 playerId = Packet->readInt(Base::Bit32);

    PlayerDataRef spPlayerData(new stPlayerStruct);

    if (0 == spPlayerData)
    {
        return false;
    }

    spPlayerData->ReadData(Packet);

    if (spPlayerData->BaseData.PlayerId != playerId)
    {
        return false;
    }

    g_Log.WriteLog("收到载入的玩家角色数据[%d] saveuid[%d]",playerId,spPlayerData->BaseData.SaveUID);

    if (g_waitDataLoading)
    {
        assert(playerId == g_curLoading);
        g_testDatas.insert(std::make_pair(playerId,*(spPlayerData.get())));

        //初始化version为1
        g_versionMap[playerId] = spPlayerData->BaseData.SaveUID;

        RETEST;
    }
    else
    {
        g_testDatas[playerId] = *(spPlayerData.get());
    }

    return true;
}

void _TestWorldDBX(void)
{
    static int testCount = 10000;

    if (g_isTestWorldDBX)
    {
        if (NetFNList[WW_LOADPLAYER_RESPOND_FROM_WORLDBX] != HandleWWLoadPlayerRespondFromWorldDBX)
        {
            NetFNList[WW_LOADPLAYER_RESPOND_FROM_WORLDBX] = HandleWWLoadPlayerRespondFromWorldDBX;
        }
    }
    else
    {
        if (NetFNList[WW_LOADPLAYER_RESPOND] != HandleWWLoadPlayerRespondFromWorldDBX)
        {
            NetFNList[WW_LOADPLAYER_RESPOND] = HandleWWLoadPlayerRespondFromWorldDBX;
        }
    }
    
    //测试数据保存到worlddbx
    if (0 == samplers.size())
    {
        if (g_isTestWorldDBX)
        {
            samplers.push_back(std::make_pair(2900,50000018));
            samplers.push_back(std::make_pair(2901,50000019));
            samplers.push_back(std::make_pair(2902,50000020));
            samplers.push_back(std::make_pair(2903,50000021));
        }
        else
        {
            samplers.push_back(std::make_pair(23,50000019));
            samplers.push_back(std::make_pair(18,50000020));
        }

        datas.resize(samplers.size());
    }

    //-----------------------------------------------------------------
    U32 playerSize = sizeof(stPlayerStruct);

    if (0 == g_worldDBSocket)
    {
        g_worldDBSocket = SERVER->GetServerManager()->GetServerSocket(g_isTestWorldDBX ? SERVICE_WORLDDBX : SERVICE_WORLDDB);

        if(0 == g_worldDBSocket)
        {
            g_Log.WriteError("与WorldDBX服务器断开连接,无法保存玩家的数据");
            return;
        }
    }

    if (g_waitDataLoading && samplers.size() == g_versionMap.size())
    {
        g_waitDataLoading = false;
        datas.clear();

        for (stdext::hash_map<U32,stPlayerStruct>::iterator iter = g_testDatas.begin(); iter != g_testDatas.end(); ++iter)
        {
            datas.push_back(iter->second);
        }

        g_Log.WriteLog("Testing started ....");
    }

    if (g_waitDataLoading)
    {   
        Sleep(1);

        for (size_t i = 0; i < samplers.size(); ++i)
        {
            U32 accountId = samplers[i].first;
            U32 playerId  = samplers[i].second;

            if (g_testDatas.end() != g_testDatas.find(playerId))
                continue;

            if (g_curLoading != playerId)
            {
                g_Log.WriteLog("开始载入测试玩家[%d]的数据...",playerId);

                char buf[64];
                Base::BitStream sendPacket(buf,sizeof(buf));
                stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WW_LOADPLAYER_REQUEST,playerId,SERVICE_WORLDDB);

                sendPacket.writeInt(accountId,Base::Bit32);
                sendPacket.writeInt(playerId,Base::Bit32);

                pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
                SERVER->GetServerSocket()->Send(g_worldDBSocket,sendPacket);

                g_curLoading = playerId;
                break;
             }
        }

        return;
    }

    bool isTesting = false;

    U32 curTime = time(0);

    for (size_t i = 0; i < datas.size(); ++i)
    {
        stPlayerStruct& tPlayer = datas[i];
        
        U32 playerId = tPlayer.BaseData.PlayerId;

        if (0 == playerId)
            continue;

        U32 checkCount = g_checkCount[playerId];

        if (checkCount >= testCount)
            continue;

        isTesting = true;

        U32 runtime = g_timerMap[playerId];

        if (0 != runtime && curTime < runtime)
            continue;

        U32 status = g_checkStatus[playerId];
        
        if (status == E_CHECK_NONE || status == E_CHECK_SAVEDATA)
        {
            g_Log.WriteLog("保存玩家[%d]的数据",playerId);

            //随机的保存数据
            __ChangePlayerData(tPlayer);

            //建立数据包
            char buf[MAX_PLAYER_STRUCT_SIZE];
            Base::BitStream sendPacket(buf,sizeof(buf));
            stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WS_SAVEPLAYER_REQUEST,playerId,SERVICE_WORLDDBX,CURRENT_PLAYER_DATA_VERSION);

            sendPacket.writeInt(tPlayer.BaseData.AccountId,Base::Bit32);
            sendPacket.writeInt(tPlayer.BaseData.SaveUID,Base::Bit32);
            sendPacket.writeInt(0,Base::Bit32);
            sendPacket.writeInt(tPlayer.BaseData.SaveUID,Base::Bit32);
            sendPacket.writeInt(0,Base::Bit32);
            sendPacket.writeInt(0,Base::Bit32);
            sendPacket.writeFlag(false);
            sendPacket.writeInt(0,Base::Bit32);
            sendPacket.writeFlag(false);

            //压缩玩家数据并发送
            g_compressBuf.size = MAX_PLAYER_STRUCT_SIZE;
            int error = SXZ_COMPRESS(g_compressBuf.buf, &g_compressBuf.size,&tPlayer,playerSize);

            if (error || g_compressBuf.size >= MAX_PACKET_SIZE)
            {
                assert(0);
            }
        
            sendPacket.writeInt(g_compressBuf.size,32);
            sendPacket.writeBits(g_compressBuf.size*8,g_compressBuf.buf);

            pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
            SERVER->GetServerSocket()->Send(g_worldDBSocket,sendPacket);

            //决定是否进行数据的检测
            int randD = rand() % 1000;

            if (randD > 990)
            {
                g_checkStatus[playerId] = E_CHECK_SAVEDATA;
                g_timerMap[playerId] = curTime + rand() % 10;

            }
            else if (randD > 800)
            {
                g_checkStatus[playerId] = E_CHECK_CHECKDATA;
                g_testDatas.erase(playerId);

                char buf[64];
                Base::BitStream sendPacket(buf,sizeof(buf));
                stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket,WW_LOADPLAYER_REQUEST,playerId,SERVICE_WORLDDB);

                sendPacket.writeInt(tPlayer.BaseData.AccountId,Base::Bit32);
                sendPacket.writeInt(playerId,Base::Bit32);

                pSendHead->PacketSize = sendPacket.getPosition() - sizeof( stPacketHead );
                SERVER->GetServerSocket()->Send(g_worldDBSocket,sendPacket);
            }
        }

        if (status == E_CHECK_CHECKDATA)
        {
            if (g_testDatas.end() != g_testDatas.find(playerId))
            {
                g_Log.WriteLog("正在比较玩家[%d]的数据",playerId);

                //检测数据是否一致
                if (0 != memcmp(&tPlayer,&g_testDatas[playerId],playerSize))
                {
                    //数据有错误！
                    assert(0);
                }

                //继续数据保存
                g_checkStatus[playerId] = E_CHECK_SAVEDATA;
            }
        }
    }

    if (isTesting)
    {
        RETEST;
    }
    else
    {
        g_Log.WriteLog("所有测试完成");
    }
}


CmdFunction(TestWorldDBX)
{
    /*g_waitchecked = false;
    g_waitDataLoading = true;
    g_curLoading  = 0;
    g_testDatas.clear();
    g_versionMap.clear();
    g_checkCount.clear();
    g_timerMap.clear();
    samplers.clear();
    datas.clear();
    g_checkStatus.clear();*/

    _TestWorldDBX();
}

#endif 