#include "stdafx.h"
#include <time.h>
#include <assert.h>
#include <sstream>
#include "DBLib/dbLib.h"
#include "WinTcp/dtServerSocket.h"
#include "WinTcp/AsyncSocket.h"
#include "Common/DumpHelper.h"
#include "zlib/zlib.h"
#include "base/SafeString.h"
#include "Common/MemGuard.h"
#include "PlayerMgr.h"
#include "WorldServer.h"
#include "base/WorkQueue.h"
#include "DBLayer\Data\TBLPlayer.h"
#include "common/dbStruct.h"
#include "DBLayer/Data/TBLAccount.h"
#include "ServerMgr.h"
#include "ChatHelper.h" 
#include "QuickLz/quicklz.h"
#include "SocialManager.h"
#include "DB_QueryPlayerCount.h"
#include "DB_CreatePlayer.h"
#include "DB_SetAccountStatus.h"
#include "SafePacket.h"


#include "Ansy_WriteAccount.h"
#include <atltime.h>

#include <algorithm>

using namespace std;

U32 g_maxLinePlayers = 1000;//最大在线人数

CPlayerManager::CPlayerManager()
{
	m_createdPlayerCount    = 0;
    m_lastUpdateTime        = 0;
    m_playerEnterInterval   = 120;

    m_30Timer.Start();
    m_60Timer.Start();

    m_canWriteAccout  = true;
}

CPlayerManager::~CPlayerManager()
{
    //因为玩家的类是智能指针,所以在这里要主动调用释放函数
    for (HashAccountMap::iterator iter = m_AccountMap.begin(); iter != m_AccountMap.end();++iter)
    {
        //SM_ACCOUNT_DATA* pData = 0;
        //iter->second->InitSM(pData);
    }

    m_PlayerNameMap.clear();
    m_PlayerMap.clear();
    m_AccountMap.clear();
    m_accountNameMap.clear();

    m_lastUpdateTime = 0;
}

bool CPlayerManager::AnsyGetPlayerData(int PlayerId,SimplePlayerData& data)
{
    DO_LOCK(Lock::Player);

    PlayerSimpleDataRef spData = GetPlayerData(PlayerId);

    if (!spData)
        return false;

    data = *(spData.get());
    return true;
}

bool CPlayerManager::AnsyGetPlayerData(const char* name,SimplePlayerData& data)
{
    DO_LOCK(Lock::Player);

    PlayerSimpleDataRef spData = GetPlayerData(name);

    if (!spData)
        return false;

    data = *(spData.get());
    return true;
}

bool CPlayerManager::AnsyGetOnlinePlayer(int playerId,AnsyPlayerInfo& data)
{
    DO_LOCK(Lock::Player);

    AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(playerId);

    if (!pAccount)
        return false;

    data.accountId = pAccount->GetAccountId();
    data.playerId  = playerId;
    data.socket    = pAccount->GetGateSocketId();
    data.isInGame  = pAccount->IsInGame();
    data.gmFlag    = pAccount->isGM;

    return true;
}

bool CPlayerManager::AnsyGetOnlinePlayer(const char* name,AnsyPlayerInfo& data)
{
    DO_LOCK(Lock::Player);

    AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(name);

    if (!pAccount)
        return false;

    data.accountId = pAccount->GetAccountId();
    data.playerId  = pAccount->GetPlayerId();
    data.socket    = pAccount->GetGateSocketId();
    data.isInGame  = pAccount->IsInGame();
    data.gmFlag    = pAccount->isGM;

    return true;
}

void CPlayerManager::LoadSimpePlayerDatas(int timeVal)
{
    g_Log.WriteLog("开始读取玩家的简单信息[%d]",timeVal);

    time_t startTime = time(0);
    
    CDBOperator db_player(SERVER->GetActorDB()->GetPool());
    M_SQL(db_player.GetConn(),"SELECT top(1000000) PlayerID FROM Tbl_Player");

    TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());

    while(db_player.GetConn()->More())
    {
        int playerId = db_player.GetConn()->GetInt();
        
        PlayerSimpleDataRef spData = PlayerSimpleDataRef(new SimplePlayerData);
        DBError err = tb_player.Load_Simple(playerId,spData.get());

        if(err == DBERR_NONE)
        {
            m_simplePlayerDatas.Add(spData->GetId(),spData->PlayerName,spData);
        }

        time_t endTime = time(0);

        if (endTime - startTime > timeVal)
        {
            break;
        }
    }
    
    time_t endTime = time(0);
    g_Log.WriteLog("结束读取玩家的简单信息[%d],timecost[%d]",timeVal,(int)(endTime - startTime));
}

void CPlayerManager::RemovePlayer(U32 playerId,const char* name)
{
	g_Log.WriteLog("移除角色数据[%d]",playerId);

	DO_LOCK( Lock::Player );

	if (m_PlayerMap.end() != m_PlayerMap.find(playerId))
	{
		m_PlayerMap.erase(playerId);
		m_PlayerNameMap.erase(name);
	}
}

void CPlayerManager::RemoveAccount( int nAccountId )
{
	char buf[256];
	sprintf_s( buf, 256, "移除帐号数据[%d]", nAccountId );
	g_Log.WriteLog( buf );

	DO_LOCK( Lock::Player );

	HashAccountMap::iterator ita = m_AccountMap.find(nAccountId);

	if(ita == m_AccountMap.end())
		return ;

	AccountRef paInfo = ita->second;

	paInfo->LastTime	= (int)time(NULL);
	paInfo->CurPlayerPos= -1;
	
	for(int i=0;i<paInfo->PlayerNum;i++)
	{
		g_Log.WriteLog("ChatLog:RemoveAccount player[%d, %s].", paInfo->m_playerIdList[i].playerID, paInfo->GetPlayerName(paInfo->m_playerIdList[i].playerID));
		RemovePlayer(paInfo->m_playerIdList[i].playerID,paInfo->GetPlayerName(paInfo->m_playerIdList[i].playerID));
	}

}

// 检查帐号的UID
ERROR_CODE CPlayerManager::VerifyUID(T_UID UID,int AccountId)
{
	DO_LOCK( Lock::Player );

	HashAccountMap::iterator ita = m_AccountMap.find(AccountId);
	if(ita == m_AccountMap.end())
        return ACCOUNT_ID_ERROR;

    AccountRef pAccountInfo = ita->second;

    if(pAccountInfo->UID > UID)
        return GAME_UID_ERROR;

    return NONE_ERROR;
}

//创建玩家角色
ERROR_CODE CPlayerManager::CreatePlayer(T_UID UID,int AccountId,const char *PlayerName,int Sex,int Body,int Face,int Hair,int HairCol,U32 ItemId,int FaceImage,int Family,int ZoneId,int playerId)
{
	AccountRef pAccountInfo;

	{
		DO_LOCK( Lock::Player );

		HashAccountMap::iterator ita = m_AccountMap.find(AccountId);

		if(ita == m_AccountMap.end())
			return ACCOUNT_ID_ERROR;
			
		pAccountInfo = ita->second;	
		if(pAccountInfo->UID!=UID)
			return GAME_UID_ERROR;

		if(pAccountInfo->PlayerNum >= MAX_CREATE_ACTOR_COUNT)
			return PLAYER_CREATE_FULL;
	}

	//初始化新角色数据
	PlayerSimpleDataRef spPlayerData(new SimplePlayerData);

	spPlayerData->AccountId  = AccountId;
	spPlayerData->PlayerId   = playerId;
	dStrcpy(spPlayerData->PlayerName, 32, PlayerName);
	spPlayerData->ZoneId	  = ZoneId;
	spPlayerData->Sex		  = (U8)Sex;
	spPlayerData->Family	  = Family;
	spPlayerData->FirstClass = Face;
	//spPlayerData->MainData.EquipInfo[5].ItemID = (U32)ItemId;
	
	DB_CreatePlayer* pDBHandle = new DB_CreatePlayer;

    pDBHandle->m_playerPos      = pAccountInfo->PlayerNum;
    pDBHandle->m_spData         = spPlayerData;
    pDBHandle->m_accountStatus  = pAccountInfo->status;
    pDBHandle->m_isOpenedTime   = SERVER->GetServerManager()->IsOpenTime(time(0));

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);

	pAccountInfo->PlayerNum++;

	return NONE_ERROR;
}

//删除玩家角色
ERROR_CODE CPlayerManager::DeletePlayer(T_UID UID,int AccountId,int playerId,bool isFromZone)
{
	DO_LOCK(Lock::Player);
	
	HashAccountMap::iterator ita = m_AccountMap.find(AccountId);

	if(ita == m_AccountMap.end())
		return ACCOUNT_ID_ERROR;

	AccountRef pAccountInfo = ita->second;

	if(pAccountInfo->UID != UID)
		return GAME_UID_ERROR;

	int CurPos = -1;

	for(int i=0; i < pAccountInfo->PlayerNum; i++)
	{
		if(pAccountInfo->m_playerIdList[i].playerID == playerId)
		{
			CurPos = i;
			break;
		}
	}

	if(CurPos == -1)
		return PLAYER_NOFOUND;
	
	PlayerSimpleDataRef spd = GetPlayerData(playerId);

	if (!spd)
		return UNKNOW_ERROR;

    /*DB_DeletePlayer* pDBHandle = new DB_DeletePlayer;

    pDBHandle->m_accountId  = AccountId;
    pDBHandle->m_playerId   = playerId;
    pDBHandle->m_isDeleteRecord = spd->Level < 10 ? true : false;
    pDBHandle->m_isFromZone = isFromZone;

    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);*/

	return NONE_ERROR;
}

void CPlayerManager::OnPlayerDeleted(U32 accountId,U32 playerId)
{
    DO_LOCK(Lock::Player);

    AccountRef spAccount = GetAccount(accountId);

    if (!spAccount)
        return;

    m_PlayerMap.erase(playerId);
	m_PlayerNameMap.erase(spAccount->GetPlayerName(playerId));

    int CurPos = -1;

	for(int i = 0; i < spAccount->PlayerNum; ++i)
	{
		if(spAccount->m_playerIdList[i].playerID == playerId)
		{
			CurPos = i;
			break;
		}
	}

	if(CurPos == -1)
		return;

	//清理角色管理
	for(int i = CurPos;i < spAccount->PlayerNum-1; i++)
	{
		spAccount->m_playerIdList[i]  = spAccount->m_playerIdList[i+1];
	}

	spAccount->m_playerIdList[spAccount->PlayerNum - 1].Clear();
	spAccount->PlayerNum--;

    //清理社会关系
	{
		MAKE_WORKQUEUE_PACKET(sendPacket, 256);
		sendPacket.writeInt(playerId, Base::Bit32);
		SEND_WORKQUEUE_PACKET(sendPacket, WQ_SOCIAL, OWQ_DistorySocialLink);
	}

    if (spAccount->PlayerNum <= 0 && (spAccount->status & EAS_PRECREATEREWARD))
    {
        //清除玩家的新建角色礼包
        spAccount->status &= ~EAS_PRECREATEREWARD;

        DB_SetAccountStatus* pDBHandle = new DB_SetAccountStatus;
        pDBHandle->m_accountId  = spAccount->GetAccountId();
        pDBHandle->m_status     = spAccount->status;
        SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
    }
}

int CPlayerManager::AddPlayerMap(AccountRef pAccountInfo,U32 playerId,const char* name)
{
	DO_LOCK( Lock::Player );
   
	//建立映射
	m_PlayerMap[playerId] = pAccountInfo;
	m_PlayerNameMap[name] = pAccountInfo;

    CHAT_HELPER->addAccountPlayer(playerId,pAccountInfo->GetAccountId(),name,pAccountInfo->GetAccountName(),pAccountInfo->GetGateSocketId());

	return NONE_ERROR;
}

void CPlayerManager::Update(void)
{
	DO_LOCK(Lock::Player);

    static int today = 0;
	int curTime = (int)time(0);

    CTime timeVal(curTime);

    if (0 == today)
    {
        //先尝试读取配置中的时间
        char strTime[64] = {0};
        GetPrivateProfileStringA("Server","lastDayTime","",strTime,sizeof(strTime),"./Config.ini");
        U32 lastDayTime = atoi(strTime);

        CTime timeStart(lastDayTime ? lastDayTime : curTime);
        today = timeStart.GetDay();
    }

    if (timeVal.GetDay() - today != 0)
    {
        today = timeVal.GetDay();
        
        OnNewDay();
    }

    m_lastUpdateTime = curTime;

    for (HashAccountMap::iterator iter = m_AccountMap.begin(); iter != m_AccountMap.end();)
    {
        AccountRef pAccountInfo = iter->second;

		if (!pAccountInfo)
        {
            ++iter;
			continue;
        }
        
		pAccountInfo->Update(curTime);
		
		if (pAccountInfo->IsOffline())
        {
			pAccountInfo->ClrBit();
			RemoveAccount(pAccountInfo->AccountId);
            m_accountNameMap.erase(pAccountInfo->GetAccountName());
			iter = m_AccountMap.erase( iter );
        }
		else
        {
			iter++;
        }
    }

	if (m_30Timer.CheckTimer())
    {
        //向Account发送当前玩家状态
        char buf[1024];
        Base::BitStream sendPacket(buf,sizeof(buf));
        stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket,"WORLD_REMOTE_PlayerCountRequest",SERVER->currentAreaId,SERVICE_REMOTESERVER);
        
        sendPacket.writeInt(m_createdPlayerCount,Base::Bit32);
        sendPacket.writeInt(SERVER->GetPlayerManager()->GetOnlineAccountCount(),Base::Bit32);
        sendPacket.writeInt(SERVER->GetServerManager()->GetMaxPlayerCount(),Base::Bit32);

        pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
        CAsyncSocket2* pSocket = SERVER->GetRemoteAccountSocket();

        if(pSocket)
        {
			pSocket->Send(sendPacket);
		}
        
        if (GetOnlineAccountCount() < SERVER->GetServerManager()->GetMaxPlayerCount())
        {
            //通知帐号服务器角色退出
            char buf[256];
            Base::BitStream sendPacket(buf,sizeof(buf));
            stPacketHead* pHead = IPacket::BuildPacketHead( sendPacket,"WORLD_ACCOUNT_PlayerLogout",0,SERVICE_ACCOUNTSERVER);
            pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);

            int svrSocket = SERVER->GetServerManager()->GetServerSocket(SERVICE_ACCOUNTSERVER);

            if(svrSocket > 0)
            {
                SERVER->GetServerSocket()->Send(svrSocket,sendPacket);
            }
        }
	}
	
	if (m_60Timer.CheckTimer())
	{
		writeAccountInfo();

		//输出下内存耗用
		DumpUsedMemory();

		//输出REMOTE ACCOUNT上的队列长度
		if (0 != SERVER->GetRemoteAccountSocket())
		{
			U32 s1 = SERVER->GetRemoteAccountSocket()->GetOutputBufferSize();
			U32 s2 = SERVER->GetRemoteAccountSocket()->GetBackBufferSize();
			g_Log.WriteLog("ra bufferlist output=%d,back=%d",s1,s2);
		}
	}
	
    SERVER->GetServerManager()->Update();
    SafePacket::Instance()->Update();
}

void CPlayerManager::TimeProcess(U32 bExit)
{
    //SERVER->GetWorkQueue()->PostEvent(0,0,0,0,WQ_TIMER);
    //return true;
	Update();
}

ERROR_CODE CPlayerManager::AddAccount(Player& accountInfo, Players& playerList)
{
	DO_LOCK(Lock::Player);
	
	int AccountId = accountInfo.AccountId;
    AccountRef paInfo;

	HashAccountMap::iterator ita = m_AccountMap.find(AccountId);

	if(ita != m_AccountMap.end())
	{
		//已经有玩家登录了的话覆盖他
		paInfo = ita->second;

        if (0 != paInfo->logoutTime)
        {   
            accountInfo.logoutTime = paInfo->logoutTime;
        }
	}

    if(paInfo)
	{
		paInfo->UID				   = accountInfo.UID;
		paInfo->LastTime		   = accountInfo.LastTime;
		paInfo->isAdult			   = accountInfo.isAdult;
		paInfo->loginTime		   = accountInfo.logoutTime;
        paInfo->isGM               = accountInfo.isGM;
		paInfo->loginType		   = accountInfo.loginType;
		paInfo->status			   = accountInfo.status;

		dStrcpy(paInfo->loginIP,sizeof(paInfo->loginIP),accountInfo.loginIP);

        paInfo->ReloadPlayerList(playerList, paInfo);
    
		return NONE_ERROR;
	}

    //第1次加入时读取玩家数据信息
	Player* pNewAccountInfo = new Player;
	*pNewAccountInfo = accountInfo;

	AccountRef accountRef( pNewAccountInfo );
    accountRef->ReloadPlayerList(playerList,accountRef);

	pNewAccountInfo->LastTime = (int)time(NULL);
	m_AccountMap[AccountId]   = accountRef;	
    m_accountNameMap[accountRef->GetAccountName()] = AccountId;

	return NONE_ERROR;
}

PlayerSimpleDataRef CPlayerManager::GetPlayerData(int playerId)
{
	DO_LOCK(Lock::Player);

    if (0 == playerId)
    {
        return PlayerSimpleDataRef();
    }

	PlayerSimpleDataRef spData;

	if (m_simplePlayerDatas.Load(playerId,spData))
	{
		return spData;
	}

	spData = PlayerSimpleDataRef(new SimplePlayerData);

	//从DB里加载简单玩家数据
	TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
	DBError err = tb_player.Load_Simple(playerId,spData.get());

	if(err != DBERR_NONE)
	{
		g_Log.WriteWarn("无法载入玩家[%d]简单结构",playerId);
		return PlayerSimpleDataRef();
	}

    m_simplePlayerDatas.Add(spData->GetId(),spData->PlayerName,spData);
	return spData;
}

PlayerSimpleDataRef CPlayerManager::GetPlayerData(const char* name)
{
	if (0 == name)
		return PlayerSimpleDataRef();

	PlayerSimpleDataRef spData;

	if (m_simplePlayerDatas.Load(name,spData))
	{
		return spData;
	}

	spData = PlayerSimpleDataRef(new SimplePlayerData);

	//从DB里加载简单玩家数据
	TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
	DBError err = tb_player.Load_Simple(name,spData.get());

	if(err != DBERR_NONE)
	{
		g_Log.WriteWarn("无法载入玩家[%s]简单结构",name);
		return PlayerSimpleDataRef();
	}
	
	return spData;
}

std::string CPlayerManager::GetPlayerName(int PlayerId) {
	PlayerSimpleDataRef pPlayer = GetPlayerData(PlayerId);
	if (pPlayer)
		return pPlayer->PlayerName;

	return "";
}

U32 CPlayerManager::GetFirstClass(U32 nPlayerID)
{
	PlayerSimpleDataRef pPlayer = GetPlayerData(nPlayerID);
	if (pPlayer)
		return pPlayer->FirstClass;

	return 0;
}

U32 CPlayerManager::GetFamily(U32 nPlayerID)
{
	PlayerSimpleDataRef pPlayer = GetPlayerData(nPlayerID);
	if (pPlayer)
		return pPlayer->Family;

	return 0;
}

bool CPlayerManager::AddPlayerData(PlayerSimpleDataRef spData)
{
	DO_LOCK(Lock::Player);

	if (!spData)
		return false;

	return m_simplePlayerDatas.Add(spData->GetId(),spData->PlayerName,spData);
}

bool CPlayerManager::IsPlayerOnline(int playerId)
{
	DO_LOCK( Lock::Player );

	AccountRef pInfo = GetOnlinePlayer( playerId );

	if(!pInfo)
		return false;

	if (playerId != pInfo->GetPlayerId())
		return false;

	if( pInfo->GetState() != Player::STATUS_IN_GAME)
		return false;

	return true;
}

void CPlayerManager::SendSimplePacket( int nPlayerId, const char*  cMessageType, int nDestZoneId, int nSrcZoneId )
{
    if (0 == nPlayerId)
        return;

	DO_LOCK( Lock::Player );

	AccountRef pAccountInfo = GetOnlinePlayer(nPlayerId);

	if( !pAccountInfo )
	{
		char buf[256];
		sprintf_s( buf, 256, "SendSimplePacket MessageType=%d 玩家[%d]数据为空",cMessageType, nPlayerId );
		g_Log.WriteWarn(buf);
		return ;
	}

	char buf[32];
	Base::BitStream SendPacket(buf,sizeof(buf));

	stPacketHead* pHead = IPacket::BuildPacketHead( SendPacket, cMessageType, pAccountInfo->AccountId, SERVICE_CLIENT, nDestZoneId, nSrcZoneId );
	pHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );

	SERVER->GetServerSocket()->Send( pAccountInfo->GetGateSocketId(), SendPacket );
}

void CPlayerManager::SendSimpleInfoPacket( int nPlayerId, const char* cMessageType, int nDestZoneId, int nSrcZoneId )
{
    if (0 == nPlayerId)
        return;

    DO_LOCK( Lock::Player );

    AccountRef pAccountInfo = GetOnlinePlayer( nPlayerId );

    if(!pAccountInfo)
    {
        return;
    }

    PlayerSimpleDataRef spPlayerData = GetPlayerData( nDestZoneId );

    if(!spPlayerData)
    {
        g_Log.WriteWarn("SendSimpleInfoPacket Type=%d 玩家数据为空[%d]",cMessageType,nPlayerId);
        return ;
    }

    char buf[256];
	Base::BitStream SendPacket(buf,sizeof(buf));
	stPacketHead* pHead = IPacket::BuildPacketHead( SendPacket, cMessageType, pAccountInfo->AccountId, SERVICE_CLIENT, nDestZoneId, nSrcZoneId );

	SendPacket.writeString(spPlayerData->PlayerName);
	SendPacket.writeInt(spPlayerData->PlayerId,Base::Bit32);
	SendPacket.writeInt(spPlayerData->Level,Base::Bit8);
	SendPacket.writeInt(spPlayerData->Sex, Base::Bit8);
	SendPacket.writeInt(spPlayerData->Family,Base::Bit8); 
	SendPacket.writeInt(spPlayerData->FirstClass,Base::Bit8);
    
	pHead->PacketSize = SendPacket.getPosition() - sizeof( stPacketHead );
	SERVER->GetServerSocket()->Send( pAccountInfo->GetGateSocketId(), SendPacket );
} 

AccountRef CPlayerManager::GetOnlinePlayer(int playerId)
{
	DO_LOCK( Lock::Player );

	HashAccountMap::iterator itp = m_PlayerMap.find(playerId);

	if(itp!=m_PlayerMap.end())
	{
		return itp->second;
	}
	
	//g_Log.WriteWarn("玩家[%d]已经离线,无法找到玩家",playerId);
	return AccountRef();
}

AccountRef CPlayerManager::GetOnlinePlayer(const char* playerName)
{
	DO_LOCK( Lock::Player );

	HashPlayerNameMap::iterator itp = m_PlayerNameMap.find(playerName);
	if(itp != m_PlayerNameMap.end())
		return itp->second;

	return AccountRef();
}

AccountRef CPlayerManager::GetAccount( int AccountId )
{
	DO_LOCK( Lock::Player );
	
	HashAccountMap::iterator it = m_AccountMap.find( AccountId );
	if( it != m_AccountMap.end() )
		return it->second;

	return AccountRef();
}

AccountRef CPlayerManager::GetAccount( const char* accountName)
{
    DO_LOCK( Lock::Player );
	
	HashAccountNameMap::iterator it = m_accountNameMap.find(accountName);

	if( it != m_accountNameMap.end())
		return GetAccount(it->second);

	return AccountRef();
}

void CPlayerManager::writeAccountInfo()
{
    DO_LOCK(Lock::Player);

    if (!m_canWriteAccout)
        return;

    U32 idx = 0;

    for(HashAccountMap::iterator it = m_AccountMap.begin(); it != m_AccountMap.end(); ++it)
	{
		AccountRef spAccount = it->second;

        if (!spAccount)
            continue;

		U32 playerId = spAccount->GetPlayerId();

		if (0 == playerId)
			continue;

        if (idx >= MAX_ACCOUNT_WRITE)
            continue;

        AccoutWriteInfo& info = m_accountWriteInfos[idx];

        strcpy_s(info.accountName,ACCOUNT_NAME_LENGTH,spAccount->AccountName);
        strcpy_s(info.playerName,MAX_NAME_LENGTH,spAccount->GetPlayerName(playerId));
        info.playerId  = playerId;
        info.accountId = spAccount->GetAccountId();
        info.state     = spAccount->GetState();
    
        idx++;
	}

	m_canWriteAccout = false;

    //写文件
    Ansy_WriteAccount* pHandle = new Ansy_WriteAccount;
    pHandle->m_totalCount = idx;
    pHandle->m_pAccounts  = m_accountWriteInfos;
    SERVER->GetQueryDBManager()->SendPacket(pHandle);
}

bool CPlayerManager::ClearPlayerData(U32 accountId)
{
    DO_LOCK(Lock::Player);

    AccountRef pInfo = SERVER->GetPlayerManager()->GetAccount(accountId);

    if(!pInfo)
        return true;

    return false;
}

void CPlayerManager::RegisterChat()
{
    DO_LOCK(Lock::Player);

	for (HashAccountMap::iterator iter=m_PlayerMap.begin(); iter!=m_PlayerMap.end(); ++iter)
	{
		AccountRef account = iter->second;
        CHAT_HELPER->addAccountPlayer(iter->first,account->GetAccountId(),account->GetPlayerName(iter->first),account->GetAccountName(),account->GetGateSocketId());
	}
}

bool CPlayerManager::HasOnlineAccount(U32 accountId)
{
    DO_LOCK(Lock::Player);
    return m_onlineAccounts.end() != m_onlineAccounts.find(accountId);
}

void CPlayerManager::AddOnlineAccount(U32 accountId)
{
    DO_LOCK(Lock::Player);
    m_onlineAccounts.insert(accountId);
}

void CPlayerManager::DelOnlineAccount(U32 accountId)
{
    DO_LOCK(Lock::Player);
    m_onlineAccounts.erase(accountId);
}

U32  CPlayerManager::GetOnlineAccountCount(void) const
{
    DO_LOCK(Lock::Player);
    return (U32)m_onlineAccounts.size();
}

void CPlayerManager::SetCreatedPlayerCount(U32 count)
{
    DO_LOCK(Lock::Player);
    m_createdPlayerCount = count;
}

U32 CPlayerManager::GetCreatedPlayerCount(void)
{
    return m_createdPlayerCount;
}

void CPlayerManager::UpdateLastDayTime()
{
    //记录新的一次时间
    U32 curTime = _time32(0);
    
    char strTime[64] = {0};
    sprintf_s(strTime,sizeof(strTime),"%d",curTime);
    WritePrivateProfileStringA("Server","lastDayTime",strTime,"./Config.ini");
}

void CPlayerManager::OnNewDay()
{
    UpdateLastDayTime();
}

bool CPlayerManager::CanAddGold(int PlayerId, S32 Gold)
{
	DO_LOCK(Lock::Player);

	S32 iCurGold = 0;
	PlayerSimpleDataRef spd = GetPlayerData(PlayerId);
	if(spd)
	{
		iCurGold = spd->Gold;
		if((iCurGold + Gold < 0) || (iCurGold + Gold > S32_MAX))
		{
			return false;
		}

		return true;
	}
	return false;
}

auto PlayerInfoSendToClient = [](int nPlayerID, int Gold, enOpType Type)
{
	MAKE_WORKQUEUE_PACKET(sendPacket, 128);
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WW_Player_UpdateClient_PlayerInfoMgr", nPlayerID);
	sendPacket.writeInt(Gold, Base::Bit32);
	sendPacket.writeInt(Type, Base::Bit32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SEND_WORKQUEUE_PACKET(sendPacket, WQ_NORMAL, OWQ_EnterGame);
};

bool CPlayerManager::AddGold(int PlayerId, S32 Gold, enOpType logType/*=LOG_GOLD_BUY*/)
{
	DO_LOCK(Lock::Player);

	PlayerSimpleDataRef spd = GetPlayerData(PlayerId);
	if(spd)
	{
		spd->Gold = mClamp((S32)spd->Gold + Gold, (S32)0, (S32)S32_MAX);
		TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
		DBError err = tb_player.UpdateGold(PlayerId, Gold);
		PlayerInfoSendToClient(PlayerId, Gold, logType);
	}
	return true;
}

bool CPlayerManager::SetFace(int PlayerId, S32 Val) 
{
	DO_LOCK(Lock::Player);

	PlayerSimpleDataRef spd = GetPlayerData(PlayerId);
	if (spd)
	{
		spd->FirstClass = mClamp((S32)Val, 0, S32_MAX);
		TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
		DBError err = tb_player.UpdateFirstClass(PlayerId, Val);
	}

	return true;
}

auto NotifyVIPCheck = [](int nPlayerID, int DrawGold) {
	MAKE_WORKQUEUE_PACKET(sendPacket, 128);
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, "WW_VIP_RECHARGE", nPlayerID);
	sendPacket.writeInt(DrawGold, Base::Bit32);
	pSendHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
	SEND_WORKQUEUE_PACKET(sendPacket, WQ_NORMAL, OWQ_EnterGame);
};

bool CPlayerManager::AddDrawGold(int PlayerId, S32 Gold)
{
	DO_LOCK(Lock::Player);

	PlayerSimpleDataRef spd = GetPlayerData(PlayerId);
	if (spd)
	{
		spd->DrawGold = mClamp((S32)spd->DrawGold + Gold, (S32)0, (S32)S32_MAX);
		TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
		DBError err = tb_player.UpdateDrawGold(PlayerId, Gold);
		if (DBError::DBERR_NONE == err) 
		{
			NotifyVIPCheck(PlayerId, spd->DrawGold);
		}
	}

	return true;
}

bool CPlayerManager::SetVipLv(int PlayerId, U32 VipLv)
{
	DO_LOCK(Lock::Player);

	PlayerSimpleDataRef spd = GetPlayerData(PlayerId);
	if(spd)
	{
		spd->VipLv = VipLv;
		TBLPlayer tb_player(SERVER_CLASS_NAME::GetActorDB());
		DBError err = tb_player.UpdateVipLv(PlayerId,spd->VipLv);
		return true;
	}

	return false;
}