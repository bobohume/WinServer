#include "ActivityManager.h"
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
#include "TaskMgr.h"
#include "CommLib/ToSql.h"
#include "DBLayer/Data/TBLExecution.h"
#include "../MailManager.h"
#include "TopBase.h"
#include "TopMgr.h"
#include "../ChatHelper.h"
#include "Json/json.h"
#include "../WorldServer.h"
#include "Common/LogHelper.h"
#include "Org.h"


DECLARE_SQL_UNIT(stActivity);
CActivityManager::CActivityManager()
{
	GET_SQL_UNIT(stActivity).SetName("Tbl_Activity");
	REGISTER_SQL_UNITKEY(stActivity, ID);
	REGISTER_SQL_UNIT(stActivity, Type);
	REGISTER_SQL_UNIT(stActivity, Name);
	REGISTER_SQL_UNIT(stActivity, Contend);
	REGISTER_SQL_DATETIME(stActivity, BeginTime);
	REGISTER_SQL_DATETIME(stActivity, EndTime);
	REGISTER_SQL_UNIT(stActivity, ItemId);
	REGISTER_SQL_UNIT(stActivity, ItemNum);
	REGISTER_SQL_UNIT(stActivity, Extend);
	REGISTER_SQL_DATETIME(stActivity, CreateTime);

	REGISTER_EVENT_METHOD("WW_ACTIVITY_Notify", this, &CActivityManager::onNotify);
	REGISTER_EVENT_METHOD("WW_CostACTIVITY_Notify", this, &CActivityManager::onNotify1);
	REGISTER_EVENT_METHOD("CW_ACTIVITY_Request", this, &CActivityManager::HandleActivityRequest);
	REGISTER_EVENT_METHOD("CW_DATANGUAN_Request", this, &CActivityManager::HandleTanGuanRequest);
	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CActivityManager::TimeProcess, 5 * 1000);
}

CActivityManager::~CActivityManager()
{

}

CActivityManager* CActivityManager::Instance()
{
	static CActivityManager s_Mgr;
	return &s_Mgr;
}

void CActivityManager::LoadDB()
{
	TBLExecution tHandle(SERVER->GetActorDB());
	tHandle.SetId(0);
	tHandle.SetSql(GET_SQL_UNIT(stActivity).LoadSql());
	tHandle.RegisterFunction([](int id, int error, void * pEx)
	{
		CDBConn * pHandle = (CDBConn*)(pEx);
		if (pHandle && error == NONE_ERROR)
		{
			stBuff buff;
			while (pHandle->More())
			{
				ActivityRef pData = ActivityRef(new stActivity);
				pData->ID = pHandle->GetInt();
				pData->Type = pHandle->GetInt();
				pData->Name = pHandle->GetString();
				pData->Contend = pHandle->GetString();
				pData->BeginTime = pHandle->GetTime();
				pData->EndTime = pHandle->GetTime();
				for (auto i = 0; i < MAX_ACTIVITY_NUM; ++i) {
					pData->ItemId[i] = pHandle->GetInt();
				}
				for (auto i = 0; i < MAX_ACTIVITY_NUM; ++i) {
					pData->ItemNum[i] = pHandle->GetInt();
				}
				for (auto i = 0; i < MAX_ACTIVITY_NUM; ++i) {
					pData->Extend[i] = pHandle->GetInt();
				}
				pData->CreateTime = pHandle->GetTime();
				ACTIVITYMGR->AddActivity(pData);
			}
		}
		return true;
	});
	tHandle.Commint();
}

void CActivityManager::CreateActivity(S32 Type, S32 BeginTime, S32 EndTime, S32 CreateTime, std::string Name, std::string Contend, S32 nItemId[MAX_ACTIVITY_NUM], S32 nItemNum[MAX_ACTIVITY_NUM], S32 nExtend[MAX_ACTIVITY_NUM])
{
	S32 nCurTime = time(NULL);
	if (BeginTime > EndTime || EndTime < nCurTime)
		return;

	ActivityRef pData = ActivityRef(new stActivity);
	pData->Type = Type;
	pData->BeginTime = BeginTime;
	pData->EndTime = EndTime;
	pData->CreateTime = CreateTime;
	pData->NotifyTime = nCurTime;
	pData->Name = Name;
	pData->Contend = Contend;
	for (auto i = 0; i < MAX_ACTIVITY_NUM; ++i) {
		pData->ItemId[i] = nItemId[i];
		pData->ItemNum[i] = nItemNum[i];
		pData->Extend[i] = nExtend[i];
	}

	InsertDB(pData);
}

void CActivityManager::AddActivity(ActivityRef pData) {
	AddData(pData->ID, pData);
	m_ActivityMap.insert(ACTIVITYMULTIMAP::value_type(pData->Type, pData));
}

void CActivityManager::DelActivity(ActivityRef pData) {
	for (auto itr = m_ActivityMap.begin(); itr != m_ActivityMap.end(); ++itr) {
		if (itr->second && itr->second == pData) {
			m_ActivityMap.erase(itr);
			break;
		}
	}
}

ActivityRef CActivityManager::GetActivity(S32 nType) {
	S32 nBeginTime = S32_MAX;
	S32 nCurTime = time(NULL);
	ActivityRef pData = ActivityRef();
	for (auto itr = m_ActivityMap.lower_bound(nType); itr != m_ActivityMap.upper_bound(nType); ++itr) {
		if (itr->second && itr->second->BeginTime < nBeginTime && itr->second->BeginTime <= nCurTime) {
			nBeginTime = itr->second->BeginTime;
			pData = itr->second;
		}
	}

	return pData;
}

std::vector<ActivityRef> CActivityManager::GetActivityList(S32 nType) {
	S32 nCurTime = time(NULL);
	std::vector<ActivityRef> ActivityList;
	for (auto itr = m_ActivityMap.lower_bound(nType); itr != m_ActivityMap.upper_bound(nType); ++itr) {
		if (itr->second  && itr->second->BeginTime <= nCurTime) {
			ActivityList.push_back(itr->second);
		}
	}

	return ActivityList;
}

auto GetActivityType = [](int type) {
	S32 nType = -1;
	if (type == eTopType_BattleUp) {
		nType = ACTIVITY_BATTLE;
	}
	else if (type == eTopType_CardUp) {
		nType = ACTIVITY_CARD;;
	}
	else if (type == eTopType_PrincessUp) {
		nType = ACTIVITY_PRINCESS;
	}
	else if (type == eTopType_LandUp) {
		nType = ACTIVITY_LAND;
	}
	return nType;
};

auto GetTopType = [](int type) {
	S32 nType = -1;
	if (type == ACTIVITY_BATTLE) {
		nType = eTopType_BattleUp;
	}
	else if (type == ACTIVITY_CARD) {
		nType = eTopType_CardUp;;
	}
	else if (type == ACTIVITY_PRINCESS) {
		nType = eTopType_PrincessUp;
	}
	else if (type == ACTIVITY_LAND) {
		nType = eTopType_LandUp;
	}
	return nType;
};

auto GetActivityTimeType = [](int type) {
	S32 nType = -1;
	if (type == ACTIVITY_TIME3) {
		nType = ACTIVITY_CHARGE_8;
	}
	else if (type == ACTIVITY_TIME4) {
		nType = ACTIVITY_CHARGE_12;;
	}
	else if (type == ACTIVITY_TIME5) {
		nType = ACTIVITY_CHARGE_88;
	}
	else if (type == ACTIVITY_TIME6) {
		nType = ACTIVITY_CHARGE_128;
	}
	return nType;
};

auto GetTimeType = [](int type) {
	S32 nType = -1;
	if (type == ACTIVITY_CHARGE_8) {
		nType = ACTIVITY_TIME3;
	}
	else if (type == ACTIVITY_CHARGE_12) {
		nType = ACTIVITY_TIME4;;
	}
	else if (type == ACTIVITY_CHARGE_88) {
		nType = ACTIVITY_TIME5;
	}
	else if (type == ACTIVITY_CHARGE_128) {
		nType = ACTIVITY_TIME6;
	}
	return nType;
};

auto RewardItem = [](ActivityRef pData, S32 nPlayerID, S32 nOrder) {
	if (!pData)
		return;
	std::string sContent;
	std::string sTitle;
	S32 nSenderID = (pData->Type+1) * 100;
	if (pData->Type == ACTIVITY_CHARGE_648 || pData->Type == ACTIVITY_CHARGE_328 || pData->Type == ACTIVITY_CHARGE_198 || \
		pData->Type == ACTIVITY_CHARGE_68 || pData->Type == ACTIVITY_CHARGE_30 || pData->Type == ACTIVITY_CHARGE_6) {
		S32 nMoney = 6;
		if (pData->Type == ACTIVITY_CHARGE_648) {
			nMoney = 648;
		}
		else if (pData->Type == ACTIVITY_CHARGE_328) {
			nMoney = 328;
		}
		else if (pData->Type == ACTIVITY_CHARGE_198) {
			nMoney = 198;
		}
		else if (pData->Type == ACTIVITY_CHARGE_68) {
			nMoney = 68;
		}
		else if (pData->Type == ACTIVITY_CHARGE_30) {
			nMoney = 30;
		}

		sContent = fmt::sprintf("恭喜陛下充值%d元成功，获得单笔充值活动奖励如下，请及时查收，祝您游戏愉快。", nMoney);
		sTitle = fmt::sprintf("获得单笔充值%d元活动奖励。", nMoney);
	}
	else if (pData->Type == ACTIVITY_BATTLE || pData->Type == ACTIVITY_CARD || pData->Type == ACTIVITY_PRINCESS || pData->Type == ACTIVITY_LAND) {
		sContent = fmt::sprintf("感谢万岁参与活动！您本次活动排行[%d]名，获得如下奖励，请及时查收。祝您游戏愉快。", nOrder);
		if (pData->Type == ACTIVITY_BATTLE) {
			sTitle = "国力冲榜活动奖励";
		}else if (pData->Type == ACTIVITY_PRINCESS) {
			sTitle = "后宫势力值冲榜活动奖励";
		}else if (pData->Type == ACTIVITY_LAND) {
			sTitle = "疆土值冲榜活动奖励";
		}else {
			sTitle = "大臣总战力值冲榜活动奖励";
		}
	}
	else if (pData->Type == ACTIVITY_CHARGE_8 || pData->Type == ACTIVITY_CHARGE_12 || pData->Type == ACTIVITY_CHARGE_88 || pData->Type == ACTIVITY_CHARGE_128) {
		auto RewardTeHui = [=](ActivityRef pData) {
			S32 nTimeId = GetTimeType(pData->Type);
			if (nTimeId != -1) {
				TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, nTimeId);
				if (!pTime) {
					TIMESETMGR->AddTimeSet(nPlayerID, nTimeId, pData->EndTime - time(NULL), "", 0);
					return true;
				}
			}
			return false;
		};

		if (!RewardTeHui(pData))
			return;

		sContent = fmt::sprintf("特惠礼包，请万岁爷及时查收，祝您游戏愉快。");
		sTitle = "特惠礼包";
	}else{
		sTitle = pData->Name + "奖励";
		sContent = fmt::sprintf("恭喜您获得 %s 奖励 ，请及时查收，祝您游戏愉快。", pData->Name.c_str());
		nSenderID += pData->Extend[1];
	}

	MAILMGR->sendMail(nSenderID, nPlayerID, 0, pData->ItemId, pData->ItemNum, sTitle.c_str(), sContent.c_str(), true);
};

auto GetTopRankAwardEx = [=](S32 nType) {
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
		if (nPlayerID) {
			auto ActivityList = ACTIVITYMGR->GetActivityList(GetActivityType(nType));
			for (auto itr = ActivityList.begin(); itr != ActivityList.end(); ++itr) {
				if (*itr && (*itr)->Extend[0] <= order && (*itr)->Extend[1] >= order) {
					RewardItem((*itr), nPlayerID, order);
					break;
				}
			}
		}
		++order;
	}
};

bool CActivityManager::CanActivity(int type) {
	S32 nType = GetActivityType(type);

	if (nType != -1) {
		ActivityRef pData = GetActivity(nType);
		if (pData)
			return true;
	}

	return false;
}

void CActivityManager::TimeProcess(U32 bExit)
{
	S32 nCurTime = time(NULL);
	for (auto itr = m_PlayerMap.begin(); itr != m_PlayerMap.end();)
	{
		ActivityRef& pData = itr->second;
		//活动结束
		if (pData && pData->EndTime < nCurTime)
		{
			//排行榜
			if (pData->Type == ACTIVITY_BATTLE || pData->Type == ACTIVITY_CARD || pData->Type == ACTIVITY_PRINCESS || pData->Type == ACTIVITY_LAND) {
				S32 nTopType = GetTopType(pData->Type);
				if (nTopType != -1) {
					GetTopRankAwardEx(nTopType);
					TOPMGR->Clear(nTopType);
				}
			}
			DelActivity(pData);
			//发放奖励
			DeleteDB(pData->ID);
			itr = m_PlayerMap.erase(itr);
		}
		else
		{
			++itr;
		}
	}
}

void CActivityManager::UpdateDB(ActivityRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(GET_SQL_UNIT(stActivity).UpdateSql(pData));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
}

void CActivityManager::InsertDB(ActivityRef pData)
{
	TBLExecution tHandle(SERVER->GetActorDB());
	tHandle.SetSql(fmt::sprintf("EXECUTE Sp_CreateActivity %d", pData->Type));
	tHandle.SetId(0);
	tHandle.RegisterFunction([pData](int id, int error, void* pEx)
	{
		CDBConn * pHandle = (CDBConn*)(pEx);
		if (pHandle && error == NONE_ERROR)
		{
			if (pHandle->More())
			{
				pData->ID = pHandle->GetInt();
				if (pData->ID != -1) {
					ACTIVITYMGR->AddActivity(pData);
					ACTIVITYMGR->UpdateDB(pData);
					ACTIVITYMGR->NotifyActivity(pData);
				}
			}
		}
		return true;
	});
	tHandle.Commint();
}

void CActivityManager::DeleteDB(S32 ID)
{
	DB_Execution* pHandle1 = new DB_Execution();
	pHandle1->SetSql(fmt::sprintf("INSERT INTO Tbl_ActivityDel SELECT* FROM Tbl_Activity WHERE ID = %d", ID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle1);

	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("Delete From Tbl_Activity Where ID=%d", ID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
}

void CActivityManager::SendInitToClient(U32 nPlayerID) {
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount) {
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_ACTIVITY_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(m_PlayerMap.size(), Base::Bit16);
		for (auto itr = m_PlayerMap.begin(); itr != m_PlayerMap.end(); ++itr) {
			if (sendPacket.writeFlag(itr->second != NULL)) {
				itr->second->WriteDataSimple(&sendPacket);
			}
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CActivityManager::NotifyActivity(ActivityRef pData) {
	stChatMessage msg;
	msg.btMessageType = CHAT_MSG_TYPE_LINE;
	Json::Value req;
	req["type"] = fmt::sprintf("%d", pData->Type);
	req["beginTime"] = fmt::sprintf("%d", pData->BeginTime);
	req["endTime"] = fmt::sprintf("%d", pData->EndTime);
	Json::Value req1;
	req1["activity"] = req;
	Json::FastWriter writer;
	std::string str = writer.write(req1);
	dStrcpy(msg.szMessage, str.length(), str.c_str());
	CHAT_HELPER->SendMessageToWorld(msg);
}

bool CActivityManager::onNotify(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	S32 nNotityType = Packet->readInt(Base::Bit8);
	S32 nPlayerID = Packet->readInt(Base::Bit32);
	S32 nBuyType = Packet->readInt(Base::Bit32);
	S32 nPrice = Packet->readInt(Base::Bit32);
	bool bRecharge = Packet->readFlag();
	S32 nType = -1;
	if (nNotityType == ACTIVITY_NOTIFY_GOLD) {
		if (nBuyType == 3)
			nType = ACTIVITY_CHARGE_6;
		else if (nBuyType == 4)
			nType = ACTIVITY_CHARGE_30;
		else if (nBuyType == 5)
			nType = ACTIVITY_CHARGE_68;
		else if (nBuyType == 6)
			nType = ACTIVITY_CHARGE_198;
		else if (nBuyType == 7)
			nType = ACTIVITY_CHARGE_328;
		else if (nBuyType == 8)
			nType = ACTIVITY_CHARGE_648;
		else if (nBuyType == 9)
			nType = ACTIVITY_CHARGE_8;
		else if (nBuyType == 10)
			nType = ACTIVITY_CHARGE_12;
		else if (nBuyType == 11)
			nType = ACTIVITY_CHARGE_88;
		else if (nBuyType == 12)
			nType = ACTIVITY_CHARGE_128;
	}

	if(bRecharge)
		SERVER->GetLog()->writeLog(nPlayerID, 0, "玩家充值", fmt::sprintf("%d", nPrice), TIMESETMGR->NewPlayer(nPlayerID));
	else
		SERVER->GetLog()->writeLog(nPlayerID, 0, "后台伪充值", fmt::sprintf("%d", nPrice), TIMESETMGR->NewPlayer(nPlayerID));

	if (nType != -1) {
		ActivityRef pData = GetActivity(nType);
		if (pData)
		{
			RewardItem(pData, nPlayerID, 0);
		}
	}

	PlayerBaseInfoRef pPlayer = PLAYERINFOMGR->GetData(nPlayerID);
	if (pPlayer && pPlayer->OrgId) {
		ORGMGR->AddGiftExp(pPlayer->OrgId, nPrice);
	}

	return true;
}

bool CActivityManager::onNotify1(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {

	S32 nType = pHead->SrcZoneId;
	S32 nVal = pHead->DestZoneId;
	S32 nPlayerID = pHead->Id;
	S32 nTimerID = nType + 500;

	if (nType < ACTIVITY_GOLD|| nType > ACTIVITY_PRINCE) {
		return false;
	}

	if (nType != -1) {
		ActivityRef pData = GetActivity(nType);
		if (pData)
		{
			auto pTime = TIMESETMGR->GetData(nPlayerID, nTimerID);
			if (!pTime) {
				pTime = TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, pData->EndTime - time(NULL), "", 0);
			}

			if (!pTime)
				return false;

			S32 nRearchVal = atoi(pTime->Flag.c_str());
			nRearchVal = mClamp(nRearchVal + nVal, 0, S32_MAX);
			TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, -1, fmt::sprintf("%d", nRearchVal), pTime->Flag1);
			auto ActivityList = ACTIVITYMGR->GetActivityList(nType);
			for (auto itr = ActivityList.begin(); itr != ActivityList.end(); ++itr) {
				S32 nPos = mClamp((*itr)->Extend[1], 0, 31);
				//已经领取
				if (pTime->Flag1 & BIT(nPos)) {
					continue;
				}

				if (*itr && (*itr)->Extend[0] <= nRearchVal) {
					RewardItem((*itr), nPlayerID, 0);
					pTime->Flag1 |= BIT(nPos);//领取
					TIMESETMGR->AddTimeSet(nPlayerID, nTimerID, -1, fmt::sprintf("%d", nRearchVal), pTime->Flag1);
				}
			}
		}
	}

	return true;
}

bool CActivityManager::HandleActivityRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet)
{
	S32 nType = Packet->readInt(Base::Bit8);
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(pHead->Id);
	if (pAccount) {
		S32 nError = 0;
		if (nType < ACTIVITY_CHARGE_6 || nType >= ACTIVITY_MAX) {
			nError = OPERATOR_PARAMETER_ERROR;
		}

		auto ActivityList = ACTIVITYMGR->GetActivityList(nType);
		CMemGuard Buffer(2048*10 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 2048 * 10);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_ACTIVITY_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		sendPacket.writeInt(ActivityList.size(), Base::Bit16);
		for (auto itr = ActivityList.begin(); itr != ActivityList.end(); ++itr) {
			if (sendPacket.writeFlag(*itr != NULL)) {
				(*itr)->WriteData(&sendPacket);
			}
		}
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
	return true;
}

bool CActivityManager::HandleTanGuanRequest(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	S32 nType = Packet->readInt(Base::Bit8);
	S32 nVal = Packet->readInt(Base::Bit8);
	S32 nDoubleTime = Packet->readInt(Base::Bit8);
	S32 nReword = 0;

	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(pHead->Id);
	if (pAccount) {
		S32 nPlayerID = pAccount->GetPlayerId();
		auto RewardTanGuan = [](ActivityRef pData, auto nReword, auto nPlayerID) {
			TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, ACTIVITY_TIME1);
			if (pData && pTime && pTime->Flag1 < pData->Extend[0])
			{
				/*if (pData->Extend[2] == 1) {
				SERVER->GetPlayerManager()->AddGold(nPlayerID, nReword);
				}*/
				if (pData->Extend[2] == 2) {
					PLAYERINFOMGR->AddMoney(nPlayerID, nReword);
				}
				else if (pData->Extend[2] == 3) {
					PLAYERINFOMGR->AddOre(nPlayerID, nReword);
				}
				else if (pData->Extend[2] == 6) {
					PLAYERINFOMGR->AddTroops(nPlayerID, nReword);
				}

				pTime->Flag1 += nReword;
				TIMESETMGR->AddTimeSet(nPlayerID, ACTIVITY_TIME1, -1, "", pTime->Flag1);
			}

			if (!pTime) {
				TIMESETMGR->AddTimeSet(nPlayerID, ACTIVITY_TIME1, pData->EndTime - time(NULL), "", nReword);
			}
		};

		ActivityRef pData = GetActivity(ACTIVITY_TANGUAN);
		if (pData)
		{
			if (nType == 1) {
				TimeSetRef pTime = TIMESETMGR->GetData(nPlayerID, ACTIVITY_TIME2);
				S32 nCurTime = (S32)(time(NULL));
				if (pTime && ((pTime->ExpireTime - nCurTime) <= 30 * 60 || pTime->ExpireTime < nCurTime)) {
					return false;
				}
				else if (!pTime) {
					TIMESETMGR->AddTimeSet(nPlayerID, ACTIVITY_TIME2, 31 * 60, "", 0);
				}

				nVal = mClamp(nVal, 0, 20);
				nDoubleTime = mClamp(nDoubleTime, 0, 20);
				nVal += nDoubleTime;
				nReword = mClamp(pData->Extend[1] * nVal * 10, 0, S32_MAX);
				RewardTanGuan(pData, nReword, nPlayerID);
			}
			else if (nType == 0) {
				nVal = mClamp(nVal, 0, 20);
				nDoubleTime = mClamp(nDoubleTime, 0, 20);
				nVal += nDoubleTime;
				nReword = mClamp(pData->Extend[1] * nVal, 0, S32_MAX);
				RewardTanGuan(pData, nReword, nPlayerID);
			}
		}
	}
	return true;
}

/*void ___CreateActivity(S32 Type, S32 BeginTime, std::string Name, S32 EndTime, S32 nItem0, S32 nItem1, S32 nItem2, S32 nItem3, S32 nNum0, S32 nNum1,\
S32 nNum2, S32 nNum3, S32 nExtend0, S32 nExtend1, S32 nExtend2, S32 nExtend3) {
S32 nItemId[MAX_ACTIVITY_NUM] = { nItem0, nItem1, nItem2, nItem3 };
S32 nItemNum[MAX_ACTIVITY_NUM] = { nNum0, nNum1, nNum2, nNum3 };
S32 nExtend[MAX_ACTIVITY_NUM] = { nExtend0, nExtend1, nExtend2, nExtend3 };
ACTIVITYMGR->CreateActivity(Type, BeginTime, Name, EndTime, nItemId, nItemNum, nExtend);
}*/

void CActivityManager::DeleteByID(S32 nID) {
	ActivityRef pData = GetData(nID);
	if (pData) {
		DelActivity(pData);
		DeleteDB(nID);
		m_PlayerMap.erase(nID);
	}
}

auto ParseInt = [](std::string Str, S32 nAarray[MAX_ACTIVITY_NUM]) {
	auto nSlot = 0;
	char* tokenPtr = strtok((char*)Str.c_str(), ";");
	while (tokenPtr != NULL)
	{
		nAarray[nSlot] = atoi(tokenPtr);
		nSlot = mClamp(++nSlot, 0, MAX_ACTIVITY_NUM - 1);
		tokenPtr = strtok(NULL, ";");
	}
};

void ___CreateActivity(S32 Type, S32 BeginTime, S32 EndTime, S32 CreateTime, const char* Name, const char* Contend, const char*  ItemStr, const char*  ItemNumStr, const char*  ExtendStr) {
	S32 nItemId[MAX_ACTIVITY_NUM] = { 0, 0, 0, 0 };
	S32 nItemNum[MAX_ACTIVITY_NUM] = { 0, 0, 0, 0 };
	S32 nExtend[MAX_ACTIVITY_NUM] = { 0, 0, 0, 0 };
	if (ItemStr){
		ParseInt(ItemStr, nItemId);
	}
	if (ItemNumStr) {
		ParseInt(ItemNumStr, nItemNum);
	}
	if (ExtendStr) {
		ParseInt(ExtendStr, nExtend);
	}
	ACTIVITYMGR->CreateActivity(Type, BeginTime, EndTime, CreateTime, Name, Contend, nItemId, nItemNum, nExtend);
}

void ___DelActivity(S32 nID) {
	ACTIVITYMGR->DeleteByID(nID);
}

void ___UpdateActivityByID(S32 nID, S32 Type, S32 BeginTime, S32 EndTime, S32 CreateTime, const char* Name, const char* Contend, const char*  ItemStr, const char*  ItemNumStr, const char*  ExtendStr) {
	S32 nItemId[MAX_ACTIVITY_NUM] = { 0, 0, 0, 0 };
	S32 nItemNum[MAX_ACTIVITY_NUM] = { 0, 0, 0, 0 };
	S32 nExtend[MAX_ACTIVITY_NUM] = { 0, 0, 0, 0 };
	if (ItemStr) {
		ParseInt(ItemStr, nItemId);
	}
	if (ItemNumStr) {
		ParseInt(ItemNumStr, nItemNum);
	}
	if (ExtendStr) {
		ParseInt(ExtendStr, nExtend);
	}
	ActivityRef pData = ACTIVITYMGR->GetData(nID);
	if (pData) {
		pData->Type = Type;
		pData->BeginTime = BeginTime;
		pData->EndTime = EndTime;
		pData->CreateTime = CreateTime;

		pData->Name = Name;
		pData->Contend = Contend;
		for (auto i = 0; i < MAX_ACTIVITY_NUM; ++i) {
			pData->ItemId[i] = nItemId[i];
			pData->ItemNum[i] = nItemNum[i];
			pData->Extend[i] = nExtend[i];
		}
		ACTIVITYMGR->UpdateDB(pData);
	}
}


void CActivityManager::Export(struct lua_State* L)
{
	lua_tinker::def(L, "CreateActivity", &___CreateActivity);
	lua_tinker::def(L, "DelActivity", &___DelActivity);
	lua_tinker::def(L, "UpdateActivity", &___UpdateActivityByID);
}
