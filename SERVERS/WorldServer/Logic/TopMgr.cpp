#include "BattleMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../Script/lua_tinker.h"
#include "Common/MemGuard.h"
#include "Common/mRandom.h"
#include "../PlayerMgr.h"
#include "WINTCP/dtServerSocket.h"
#include "CardMgr.h"
#include "PlayerInfoMgr.h"
#include "SkillData.h"
#include "BuffData.h"
#include "CopyData.h"
#include "TimeSetMgr.h"
#include "ItemMgr.h"
#include "TopMgr.h"
#include "DBLayer/Data/TBLExecution.h"
#include "DBLayer/Common/DBUtility.h"
#include "PvpData.h"
#include "CommLib/redis.h"
#include "PvpData.h"
#include "../MailManager.h"
#include "CommLib/format.h"
#include "platform.h"
#include "ActivityManager.h"
#include "TaskMgr.h"

TopRankConfig gs_topRankConfigs[eTopType_End];

//TODO 对每种类型进行配置
CTopManager::CTopManager(void)
{
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CTopManager::Update,  20 * 1000);
	REGISTER_EVENT_METHOD("CW_OPENTOPPVE_REQUEST", this, &CTopManager::HandleOpenPveRequest);
	REGISTER_EVENT_METHOD("CW_OPENTOPPVEWIN_REQUEST", this, &CTopManager::HandleOpenPveWinRequest);
	REGISTER_EVENT_METHOD("CW_OPENTOP_REQUEST", this, &CTopManager::HandleOpenTopRankRequest);
	REGISTER_EVENT_METHOD("CW_UPTOP_REQUEST", this, &CTopManager::HandleUpTopRequest);
	gs_topRankConfigs[eTopType_PVEWIN].playerCount = 100;
	g_TopAwardDataMgr->read();
}

auto GetTopRankAward = [](S32 nType) {
	std::vector<std::string> VecCmd;
	CRedisDB cmdDB(SERVER->GetRedisPool());
	if (TOPMGR->__isDesc(nType))
		cmdDB.Command(fmt::sprintf("zrevrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);
	else
		cmdDB.Command(fmt::sprintf("zrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);

	std::deque<int> PlayerVec;
	for (auto itr = VecCmd.begin(); itr != VecCmd.end(); ++itr)
	{
		std::string UID = *itr;
		S32 nPlayerID = atoi(UID.c_str());
		if (nPlayerID)
		{
			PlayerVec.push_back(nPlayerID);
		}
	}

	//奖励
	int order = 1;
	while (!PlayerVec.empty())
	{
		S32 nPlayerID = PlayerVec.front();
		PlayerVec.pop_front();
		if (nPlayerID > 10000) {
			CTopAwardData* pData = g_TopAwardDataMgr->getTopRand(order);
			if (pData)
			{
				std::string sContent = fmt::sprintf("恭喜万岁爷赛季校场排位赛中获得第%d名的成绩，现已将排名奖励以附件形式发放，请万岁爷及时查收，祝您游戏愉快。", order);
				MAILMGR->sendMail(0, nPlayerID, 0, pData->m_Award, pData->m_AwardNum, "赛季校场排名奖励", sContent.c_str(), true);
			}
			auto pPlayer = PLAYERINFOMGR->GetPlayer(nPlayerID);
			if (pPlayer)
				PLAYERINFOMGR->AddPveScore(nPlayerID, 1000 - pPlayer->PveScore);
		}

		++order;
	}
};

auto InitRobot = []() {
	for (auto itr = g_PvpDataMgr->m_PvpMap.begin(); itr != g_PvpDataMgr->m_PvpMap.end(); ++itr) 
	{
		if (itr->second) 
		{
			TOPMGR->NewInData(eTopType_PVE, itr->second->m_ID, itr->second->m_Score);
		}
	}
};

std::string StrReplace(std::string Str, std::string FindStr, std::string RpStr)
{
	std::string::size_type pos = 0;
	while ((pos = Str.find(FindStr.c_str(), pos)) != std::string::npos)
	{
		Str.replace(pos, FindStr.length(), RpStr.c_str());
	}
	return Str;
}


CTopManager::~CTopManager(void)
{
	g_TopAwardDataMgr->close();
}

CTopManager* CTopManager::Instance()
{
	static CTopManager s_Mgr;
	return &s_Mgr;
}

auto GetTopPlayerName = [](S32 nPlayerID)
{
	if (nPlayerID < 10000)
	{
		CPvpData* pData = g_PvpDataMgr->getData(nPlayerID);
		if (pData)
		{
			return (const char*)pData->m_sName.c_str();
		}
	}
	else
	{
		PlayerSimpleDataRef pPlayer = SERVER->GetPlayerManager()->GetPlayerData(nPlayerID);
		if (pPlayer)
		{
			return (const char*)pPlayer->PlayerName;
		}
	}
	return "";
};

void stTopRank::WriteDataEx(Base::BitStream* stream)
{
	stream->writeString(Util::MbcsToUtf8(Name), 50);
	stream->writeInt(atoi(UID.c_str()), Base::Bit32);
	stream->writeInt(LastTime, Base::Bit32);
	stream->writeInt(Val[0], Base::Bit32);
	std::string strName = GetTopPlayerName(Val[1]);
	stream->writeString(Util::MbcsToUtf8(strName), 50);
}

void stTopRank::WriteDataEx(Base::BitStream* stream, int order)
{
	stream->writeString(Util::MbcsToUtf8(Name), 50);
	stream->writeInt(order, Base::Bit32);
	stream->writeInt(Val[0], Base::Bit32);
	stream->writeInt(Val[1], Base::Bit32);
	stream->writeInt(Val[2], Base::Bit32);
}

void CTopManager::LoadDB()
{
	TBLExecution tHandle(SERVER->GetActorDB());;
	tHandle.SetId(0);
	tHandle.SetSql(fmt::sprintf("SELECT UID, Type, Val0, Val1, Val2, LastTime FROM Tbl_TopRank ORDER BY Val0 DESC"));
	tHandle.RegisterFunction([](int id, int error, void * pEx)
	{
		CDBConn * pHandle = (CDBConn*)(pEx);
		if (pHandle && error == NONE_ERROR)
		{
			stBuff buff;
			bool bInit = true;
			while (pHandle->More())
			{
				TopRankRef pData = TopRankRef(new stTopRank);
				pData->UID = pHandle->GetString();
				if (pData->UID == " ")
					pData->UID = "";
				pData->Type = pHandle->GetInt();
				for(int i = 0; i < 3; ++i)
					pData->Val[i] = pHandle->GetInt();
				pData->LastTime = pHandle->GetTime();
				CRedisDB conn(SERVER->GetRedisPool());
				conn.Command(fmt::sprintf("zadd Tbl_TopRank_%d %d %s", pData->Type, pData->Val[0], pData->UID.c_str()));
				TOPMGR->InsertData(pData);
				bInit = false;
			}

			if (bInit) {
				InitRobot();
			}
		}
		return true;
	});
	tHandle.Commint();
}

bool CTopManager::UpdateDB(int type, TopRankRef pData)
{
	char szLoginTime[32] = { 0 };
	unsigned long curTime = (unsigned long)time(NULL);
	CRedisDB conn(SERVER->GetRedisPool());
	conn.Command(fmt::sprintf("zadd Tbl_TopRank_%d %d %s", type, pData->Val[0], pData->UID.c_str()));
	GetDBTimeString(curTime, szLoginTime);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("UPDATE Tbl_TopRank	 \
		SET Val0=%d,		 Val1=%d,		Val2=%d,		LastTime='%s'		WHERE UID='%s' AND	Type=%d", \
		pData->Val[0],	 pData->Val[1], pData->Val[2],	szLoginTime,	pData->UID.c_str(), type));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CTopManager::InsertDB(std::string UID, int type, TopRankRef pData)
{
	char szLoginTime[32] = { 0 };
	unsigned long curTime = (unsigned long)time(NULL);
	CRedisDB conn(SERVER->GetRedisPool());
	conn.Command(fmt::sprintf("zadd Tbl_TopRank_%d %d %s", type, pData->Val[0], pData->UID.c_str()));
	GetDBTimeString(curTime, szLoginTime);
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("INSERT INTO  Tbl_TopRank \
		(UID,			Type,			Val0,			Val1,			Val2,			LastTime) \
		VALUES('%s',	%d,				%d,				%d,				%d,				'%s')",  \
	pData->UID.c_str(), type,	pData->Val[0],  pData->Val[1],  pData->Val[2],  szLoginTime));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CTopManager::DeleteDB(std::string UID, int type)
{
	CRedisDB conn(SERVER->GetRedisPool());
	conn.Command(fmt::sprintf("zrem Tbl_TopRank_%d %s", type, UID.c_str()));
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE  Tbl_TopRank WHERE UID='%s' AND Type=%d", \
		UID.c_str(), type));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

void CTopManager::DeleteDB(int type)
{
	CRedisDB conn(SERVER->GetRedisPool());
	conn.Command(fmt::sprintf("zremrangebyrank Tbl_TopRank_%d 0 -1", type));
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetId(0);
	pHandle->SetSql(fmt::sprintf("DELETE Tbl_TopRank WHERE TYPE=%d", type));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
}

void CTopManager::DeletePlayerFromDB(int playerId, int type)
{	
	std::ostringstream oStr;
	oStr << playerId;
	DeleteDB(oStr.str().c_str(), type);
}

void CTopManager::Clear(U32 type)
{
	if (type >= eTopType_End)
		return;

	TOPRANKITEMS& items = m_topRanks[type];
	//从内存中删除数据
	items.clear();

	//从数据库中删除
	DeleteDB(type);
}

void CTopManager::DelTop(std::string UID, int type)
{
	DO_LOCK(Lock::Top);

	if (-1 == type)           //删除所有类型榜单
	{
		for (int i = 0; i < eTopType_End; i++)
		{
			TOPRANKITEMS& items = m_topRanks[i];

			//删除前台榜单
			items.erase(UID);
			DeleteDB(UID, i);
		}
	}
	else
	{
		TOPRANKITEMS& items = m_topRanks[type];
		//删除前台榜单
		items.erase(UID);
		DeleteDB(UID, type);
	}
}

void CTopManager::InsertData(TopRankRef pData)
{
	pData->Name = GetTopPlayerName(atoi(pData->UID.c_str()));
	TOPRANKITEMS& items = m_topRanks[pData->Type];
	items.insert(TOPRANKITEMS::value_type(pData->UID, pData));
}

bool CTopManager::isSpecial(int type)
{
	return false;
}

bool CTopManager::__isDesc(int type)
{
	if (type == eTopType_PVEWIN)
		return false;

	return true;
}

int CTopManager::getRankForPlayer(int type, int playerId)
{
	std::ostringstream oStr;
	oStr << playerId;
	std::string strRank;
	CRedisDB cmdDB(SERVER->GetRedisPool());
	if (__isDesc(type))
		strRank = cmdDB.Command(fmt::sprintf("zrevrank Tbl_TopRank_%d %s", type, oStr.str().c_str()));
	else
		strRank = cmdDB.Command(fmt::sprintf("zrank Tbl_TopRank_%d %s", type, oStr.str().c_str()));
	return atoi(strRank.c_str());
}

TopRankRef CTopManager::GetData(int type, std::string UID)
{
	TOPRANKITEMS& items = m_topRanks[type];
	TOPRANKITEMS::iterator it = items.find(UID);
	if (it != items.end())
	{
		return it->second;
	}

	return TopRankRef();
}

TopRankRef CTopManager::GetData(int type, int playerId)
{
	std::ostringstream oStr;
	oStr << playerId;
	return GetData(type, oStr.str());
}

void CTopManager::UpTop(int type, int playerId)
{
	if (type < eTopType_Battle || type > eTopType_Land)
		return;
	
	S32 nVal[4] = { 0, 0, 0, 0 };
	S32 nSlot = (type - eTopType_Battle)% 4;
	TimeSetRef pTime = TIMESETMGR->GetData(playerId, TOP_RANK_UP_TIME);
	if (pTime) {
		nVal[0] = pTime->Flag1 & 0xFF;
		nVal[1] = (pTime->Flag1 >> 8) & 0xFF;
		nVal[2] = (pTime->Flag1 >> 16) & 0xFF;
		nVal[3] = (pTime->Flag1 >> 24) & 0xFF;
	}

	if (nVal[nSlot] == 1)
		return;

	S32 nRandVal = gRandGen.randI(1, 100);
	S32 nGold = 10;
	if (nRandVal <= 50) {
		nGold = 10;
	}
	else if (nRandVal <= 80) {
		nGold = 20;
	}
	else if (nRandVal <= 95) {
		nGold = 30;
	}
	else {
		nGold = 50;
	}

	nVal[nSlot] = 1;

	S32 nFlag = (nVal[0] & 0xFF) | ((nVal[1] & 0xFF) << 8) | ((nVal[2] & 0xFF) << 16) | ((nVal[3] &0xFF) << 24);
	SERVER->GetPlayerManager()->AddGold(playerId, nGold);
	TIMESETMGR->AddTimeSet(playerId, TOP_RANK_UP_TIME, Platform::getNextTime(Platform::INTERVAL_DAY) - time(NULL), "", nFlag);
	SUBMIT(playerId, 66, 1);
}

void CTopManager::Update(U32)
{
	//static bool isDeleted = false;
	static bool isDeletedTop = false;
	tm tmCurTime;
	time_t curTime;
	time(&curTime);
	localtime_s(&tmCurTime, &curTime);

	//删除校场
	{
		if (0 == tmCurTime.tm_hour && 0 == tmCurTime.tm_wday  && !isDeletedTop)
		{
			g_Log.WriteLog("发送校场赛季奖励");
			GetTopRankAward(eTopType_PVE);
			Clear(eTopType_PVE);
			InitRobot();
			isDeletedTop = true;
			g_Log.WriteLog("发送校场赛季奖励结束,已删标识设置成功!");
		}
		if (tmCurTime.tm_hour == 1 && isDeletedTop)
		{
			isDeletedTop = false;
			g_Log.WriteLog("发送校场赛季奖励重置成功!");
		}
	}

	/*for (int iType = eTopType_Start; iType < eTopType_End; ++iType)
	{
		//meet our update time
		if (curTime - gs_topRankConfigs[iType].lastSortTime >= gs_topRankConfigs[iType].sortInterval)
		{
			gs_topRankConfigs[iType].lastSortTime = curTime;
			UpdateTop(iType);
		}
	}*/

	/*if (tmCurTime.tm_hour == 0 && !isDeleted)
	{
		g_Log.WriteLog("开始删除过期的排行榜");

		for (int iType = eTopType_Start; iType < eTopType_End; ++iType)
		{
			DeleteOverdue(iType, curTime);
		}
		isDeleted = true;

		g_Log.WriteLog("删除过期的排行榜结束,已删标识设置成功!");
	}
	if (tmCurTime.tm_hour == 1 && isDeleted)
	{
		isDeleted = false;
		g_Log.WriteLog("过期排行榜已删标识重置成功!");
	}*/

	/*static bool isClearAll = false;

	if (5 == tmCurTime.tm_hour && 1 == tmCurTime.tm_wday)
	{
		if (!isClearAll)
		{
			for (int iTopType = eTopType_Level; iTopType <= eTopType_Score; iTopType++)
			{
				Clear(iTopType);
			}
			isClearAll = true;
		}
	}

	if (6 == tmCurTime.tm_hour && 1 == tmCurTime.tm_wday)
	{
		//isClearAll = false;
	}*/
}

void CTopManager::DeleteOverdue(int iType, time_t curTime)
{
	if (!__isNeedOverDue(iType))
		return;

	TOPRANKITEMS& items = m_topRanks[iType];
	TOPRANKITEMS::iterator it = items.begin();

	while (it != items.end())
	{
		if (curTime - (it->second)->LastTime >= TR_CLEARTIME)
		{
			TOPRANKITEMS::iterator itt = it++;

			std::string  UID = itt->first;

			items.erase(itt);
			DelTop(UID, iType);
			g_Log.WriteLog("删除过期的排行榜项[type]=%d,[uid]=%s", iType, UID.c_str());

			continue;
		}

		it++;
	}
}

bool  CTopManager::__isNeedOverDue(int iType)
{
	return true;
}

void CTopManager::NewInDataEx(int type, S32 nPlayerID, S32 OldValue0, S32 Value0, S32 Value1, S32 Value2)
{
	if (!ACTIVITYMGR->CanActivity(type))
		return;

	if (Value0 < OldValue0)
		return;

	NewInData(type, nPlayerID, abs(Value0 - OldValue0), Value1, Value2);
}

void CTopManager::NewInData(int type, std::string UID, S32 Value0, S32 Value1, S32 Value2)
{
	TopRankRef pData = TopRankRef(new stTopRank);
	pData->Val[0] = Value0;
	pData->Val[1] = Value1;
	pData->Val[2] = Value2;
	pData->LastTime = _time32(NULL);
	pData->Type = type;
	pData->UID = UID;
	NewInData(type, pData);
}

void CTopManager::NewInData(int type, S32 nPlayerID, S32 Value0, S32 Value1, S32 Value2)
{
	std::ostringstream oStr;
	oStr << nPlayerID;
	NewInData(type, oStr.str(), Value0, Value1, Value2);
}

void CTopManager::UpdateTop(int iType)
{
}

void CTopManager::NewInData(int nType, TopRankRef pData)
{
	if (nType >= eTopType_Start &&  nType < eTopType_End)
	{
		{
			if (pData->Val[0] <= 0)
				return;
			//计算排行榜的值
			TopRankRef pTopData = GetData(nType, pData->UID);
			if (pTopData)
			{
				auto InsertTop = [=]() {
					for (auto i = 0; i < 3; ++i)
						pTopData->Val[i] = pData->Val[i];

					pTopData->LastTime = pData->LastTime;
					UpdateDB(nType, pTopData);
				};

				if (nType == eTopType_PVE || nType == eTopType_PVEWIN || nType == eTopType_Battle) {
					InsertTop();
				}
				else if (nType == eTopType_BattleUp || nType == eTopType_CardUp || nType == eTopType_PrincessUp || nType == eTopType_LandUp) {
					pTopData->Val[0] += pData->Val[0];
					for (auto i = 1; i < 3; ++i)
						pTopData->Val[i] = pData->Val[i];

					pTopData->LastTime = pData->LastTime;
					UpdateDB(nType, pTopData);
				}
				else {
					if (pTopData->Val[0] < pData->Val[0]) {
						InsertTop();
					}
				}
			}
			else
			{
				InsertData(pData);
				InsertDB(pData->UID, nType, pData);
			}
		}
	}
}

std::vector<std::string> CTopManager::GetRank(int type, std::string UID, S32 nValue)
{
	std::vector<std::string> UIDVec;
	TOPRANKITEMS& items = m_topRanks[type];
	S32 nTime = 1;
	S32 nOrder = 0;
	while (UIDVec.empty() && nTime < 10)
	{
		std::vector<std::string> VecCmd;
		CRedisDB cmdDB(SERVER->GetRedisPool());
		std::string cmd;
		if (__isDesc(type))
			cmd = cmdDB.Command(fmt::sprintf("zrevrank Tbl_TopRank_%d %s", type, UID.c_str()));
		else
			cmd = cmdDB.Command(fmt::sprintf("zrank Tbl_TopRank_%d %s", type, UID.c_str()));

		if (cmd == "nil"){
			cmd = cmdDB.Command(fmt::sprintf("zcount Tbl_TopRank_%d %d %d", type, 0, S32_MAX));
		}

		nOrder = atoi(cmd.c_str());
		if (__isDesc(type))
			cmdDB.Command(fmt::sprintf("zrevrange Tbl_TopRank_%d %d %d", type, ((nOrder-25) < 0 ? 0 : (nOrder - 25)), nOrder+25), VecCmd);
		else
			cmdDB.Command(fmt::sprintf("zrange Tbl_TopRank_%d %d %d", type, ((nOrder - 25) < 0 ? 0 : (nOrder - 25)), nOrder + 25), VecCmd);

		++nTime;
		if(VecCmd.empty())
			continue;

		for(auto sUID : VecCmd)
		{
			if (sUID != UID)
				UIDVec.push_back(sUID);
		}
	}
	return UIDVec;
}

std::vector<std::string> CTopManager::GetRank(int type, U32 start, U32 end)
{
	CRedisDB cmdDB(SERVER->GetRedisPool());
	std::vector<std::string> VecCmd;
	if (__isDesc(type))
		cmdDB.Command(fmt::sprintf("zrevrange Tbl_TopRank_%d %d %d", type, start, end), VecCmd);
	else
		cmdDB.Command(fmt::sprintf("zrange Tbl_TopRank_%d %d %d", type, start, end), VecCmd);

	return VecCmd;
}

bool CTopManager::HandleOpenPveRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 nType = eTopType_PVE;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		TOPRANKITEMS& items = m_topRanks[nType];
		CMemGuard Buffer(2048 * 10 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 2048 * 10);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_OPENTOPPVE_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		int order = 1;
		TopRankRef pTop = GetData(nType, pAccount->GetPlayerId());
		sendPacket.writeInt(1, Base::Bit16);
		if(sendPacket.writeFlag(pTop != NULL))
		{
			pTop->WriteData(&sendPacket, getRankForPlayer(nType, pAccount->GetPlayerId()));
		}

		CRedisDB cmdDB(SERVER->GetRedisPool());
		std::vector<std::string> VecCmd;
		if (__isDesc(nType))
			cmdDB.Command(fmt::sprintf("zrevrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);
		else
			cmdDB.Command(fmt::sprintf("zrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);

		sendPacket.writeInt((VecCmd.size() > 100 ? 100 : VecCmd.size()), Base::Bit16);
		for(auto itr  = VecCmd.begin(); itr != VecCmd.end(); ++itr)
		{
			std::string UID = *itr;
			auto iter = items.find(UID);
			if (iter != items.end() && iter->second)
			{
				sendPacket.writeFlag(true);
				iter->second->WriteData(&sendPacket, order);
				++order;
			}
			else
				sendPacket.writeFlag(false);
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CTopManager::HandleOpenPveWinRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 nType = eTopType_PVEWIN;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		TOPRANKITEMS& items = m_topRanks[nType];
		CMemGuard Buffer(2048 * 10 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 2048 * 10);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_OPENTOPPVEWIN_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		int order = 1;

		std::vector<std::string> VecCmd;
		CRedisDB cmdDB(SERVER->GetRedisPool());
		if (__isDesc(nType))
			cmdDB.Command(fmt::sprintf("zrevrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);
		else
			cmdDB.Command(fmt::sprintf("zrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);

		sendPacket.writeInt((VecCmd.size() > 100 ? 100 : VecCmd.size()), Base::Bit16);
		for (auto itr = VecCmd.begin(); itr != VecCmd.end(); ++itr)
		{
			std::string UID = *itr;
			auto iter = items.find(UID);
			if (iter != items.end() && iter->second)
			{
				sendPacket.writeFlag(true);
				iter->second->WriteDataEx(&sendPacket);
				++order;
			}
			else
				sendPacket.writeFlag(false);
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CTopManager::HandleOpenTopRankRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	S32 nType = Packet->readInt(Base::Bit8);
	if (nType < eTopType_Start || nType >= eTopType_End)
		return false;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		TOPRANKITEMS& items = m_topRanks[nType];
		CMemGuard Buffer(2048 * 10 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 2048 * 10);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_OPENTOP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		int order = 1;
		TopRankRef pTop = GetData(nType, pAccount->GetPlayerId());
		sendPacket.writeInt(1, Base::Bit16);
		if (sendPacket.writeFlag(pTop != NULL))
		{
			pTop->WriteData(&sendPacket, getRankForPlayer(nType, pAccount->GetPlayerId()));
		}
		std::vector<std::string> VecCmd;
		CRedisDB cmdDB(SERVER->GetRedisPool());
		if (__isDesc(nType))
			cmdDB.Command(fmt::sprintf("zrevrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);
		else
			cmdDB.Command(fmt::sprintf("zrange Tbl_TopRank_%d %d %d", nType, 0, 99), VecCmd);

		sendPacket.writeInt((VecCmd.size() > 100 ? 100 : VecCmd.size()), Base::Bit16);
		for (auto itr = VecCmd.begin(); itr != VecCmd.end(); ++itr)
		{
			std::string UID = *itr;
			auto iter = items.find(UID);
			if (iter != items.end() && iter->second)
			{
				sendPacket.writeFlag(true);
				iter->second->WriteDataEx(&sendPacket, order);
				++order;
			}
			else
				sendPacket.writeFlag(false);
		}

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CTopManager::HandleUpTopRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	U32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	S32 nType = Packet->readInt(Base::Bit8);
	if (pAccount)
	{
		UpTop(nType, pAccount->GetPlayerId());
		std::vector<std::string> uids = GetRank(nType, (U32)0, (U32)10);
		if (!uids.empty()) {
			S32 index = gRandGen.randI(0, uids.size() - 1);
			U32 nPlayerID = (U32)std::atoi(uids[index].c_str());
			std::string PlayerName = SERVER->GetPlayerManager()->GetPlayerName(nPlayerID);
			U32 Family = SERVER->GetPlayerManager()->GetFamily(nPlayerID);
			CMemGuard Buffer(128 MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), 128);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_UPTOP_RESPONSE", pAccount->GetAccountId(), SERVICE_CLIENT);
			sendPacket.writeInt(0, Base::Bit16);
			sendPacket.writeInt(Family, Base::Bit32);
			sendPacket.writeString(Util::MbcsToUtf8(PlayerName));
			pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}
	}
	return true;
}

void ___DelTop(int PlayerID, int type)
{
	std::ostringstream Ostr;
	Ostr << PlayerID;
	TOPMGR->DelTop(Ostr.str().c_str(), type);
}

void CTopManager::Export(struct lua_State* L) 
{
	lua_tinker::def(L, "DelTop", &___DelTop);
}