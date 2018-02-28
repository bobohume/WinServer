#include "BossMgr.h"
#include "Common/UUId.h"
#include "DBLayer/Common/Define.h"
#include "Common/MemGuard.h"
#include "../WorldServer.h"
#include "../Database/DB_Execution.h"
#include "../Script/lua_tinker.h"
#include "../Database/platformStrings.h"
#include "../PlayerMgr.h"
#include "Common/mRandom.h"
#include "PlayerInfoMgr.h"
#include "TimeSetMgr.h"
#include "CardMgr.h"
#include "BuffMgr.h"
#include "platform.h"
#include "WINTCP/dtServerSocket.h"
#include "BossData.h"
#include "BASE/tVector.h"
#include "BASE/mMathFn.h"
#include "CommLib/ToSql.h"
#include "Common/UtilString.h"
#include "Org.h"
#include "../MailManager.h"

DECLARE_SQL_UNIT(stBoss);
DECLARE_SQL_UNIT(stBattleRank);
DECLARE_SQL_UNIT(stBattleInfoDB);
#define BOSSTABLE		"Tbl_Boss"
#define BOSSRANKTABLE	"Tbl_BossRank"
#define BOSSBATTLETABLE "Tbl_BossBattle"
#define BOSSDB			GET_SQL_UNIT(stBoss)
#define BOSSRANKDB		GET_SQL_UNIT(stBattleRank)
#define BOSSBATTLEDB	GET_SQL_UNIT(stBattleInfoDB)
CBossMgr::CBossMgr()
{
	GET_SQL_UNIT(stBoss).SetName(BOSSTABLE);
	REGISTER_SQL_UNITKEY(stBoss, BossId);
	REGISTER_SQL_UNIT(stBoss, DataId);
	REGISTER_SQL_UNIT(stBoss, Hp);
	REGISTER_SQL_UNIT(stBoss, Type);
	REGISTER_SQL_DATETIME(stBoss, CreateTime);
	REGISTER_SQL_DATETIME(stBoss, EndTime);

	GET_SQL_UNIT(stBattleRank).SetName(BOSSRANKTABLE);
	REGISTER_SQL_UNITKEY(stBattleRank, BossId);
	REGISTER_SQL_UNITKEY(stBattleRank, PlayerId);
	REGISTER_SQL_UNIT(stBattleRank, Hp);

	GET_SQL_UNIT(stBattleInfoDB).SetName(BOSSBATTLETABLE);
	REGISTER_SQL_UNITKEY(stBattleInfoDB, BossId);
	REGISTER_SQL_UNITKEY(stBattleInfoDB, PlayerId);
	REGISTER_SQL_UNIT(stBattleInfoDB, CurPos);
	REGISTER_SQL_UNIT(stBattleInfoDB, UID);
	REGISTER_SQL_UNIT(stBattleInfoDB, Agi);
	REGISTER_SQL_UNIT(stBattleInfoDB, Agi_Grow);
	REGISTER_SQL_UNIT(stBattleInfoDB, Int);
	REGISTER_SQL_UNIT(stBattleInfoDB, Int_Grow);
	REGISTER_SQL_UNIT(stBattleInfoDB, Str);
	REGISTER_SQL_UNIT(stBattleInfoDB, Str_Grow);
	REGISTER_SQL_UNIT(stBattleInfoDB, Add_gPc);
	REGISTER_SQL_DATETIME(stBattleInfoDB, Add_CoolDown);
	REGISTER_SQL_DATETIME(stBattleInfoDB, CoolDown);

	SERVER->GetWorkQueue()->GetTimerMgr().notify(this, &CBossMgr::TimeProcess, 1 * 1000);
	g_BossDataMgr->read();

	REGISTER_EVENT_METHOD("CW_Boss_OpenOrg_Request", this, &CBossMgr::HandleBossOpenOrgResponse);
	REGISTER_EVENT_METHOD("CW_Boss_BuyBuff_Request", this, &CBossMgr::HandleBossBuyBuffResponse);
	REGISTER_EVENT_METHOD("CW_BossInfo_Request",	 this, &CBossMgr::HandleBossInfoResponse);
	REGISTER_EVENT_METHOD("CW_Boss_Battle_Request",	 this, &CBossMgr::HandleBossBattleResponse);
}

CBossMgr::~CBossMgr()
{
	g_BossDataMgr->clear();
}

CBossMgr* CBossMgr::Instance()
{
	static CBossMgr s_Mgr;
	return &s_Mgr;
}

BossRef	CBossMgr::GetData(S32 nID) {
	auto itr = m_BossMap.find(nID);
	if (itr != m_BossMap.end()) {
		return itr->second;
	}

	return BossRef();
}

void CBossMgr::AddData(BossRef pData) {
	m_BossMap.insert(BOSS_MAP::value_type(pData->BossId, pData));
}

enOpError CBossMgr::CreateBoss(S32 nID, S32 nDataID)
{
	auto pData = GetData(nID);
	auto pBossData = g_BossDataMgr->getData(nDataID);
	if (pData || !pBossData)
	{
		return OPERATOR_BOSS_NO;
	}

	pData = BossRef(new stBoss);
	pData->BossId = nID;
	pData->CreateTime = time(NULL);
	pData->DataId = nDataID;
	pData->EndTime = pData->CreateTime + 30 * 60;
	pData->Hp = pBossData->m_Hp;
	pData->Type = gRandGen.randI(0, 2);//(政治、学识、统率)
	AddData(pData);
	InsertDB(pData);
	return OPERATOR_NONE_ERROR;
}

enOpError CBossMgr::BattleBoss(S32 nPlayerID, S32 nID, std::string UID[3])
{
	auto itr = m_BossMap.find(nID);
	if (itr == m_BossMap.end()) {
		return OPERATOR_BOSS_NO;
	}

	auto itr1 = m_BossBattleMap.find(nID);
	if (itr1 != m_BossBattleMap.end()) {
		auto itr2 = itr1->second.find(nPlayerID);
		if (itr2 != itr1->second.end()) {
			return OPERATOR_BOSS_PLAYER_EXIST;
		}
	}

	std::vector<CardRef> CardVec;
	for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
		auto pCard = CARDMGR->GetData(nPlayerID, UID[i]);
		if (pCard) {
			CardVec.push_back(pCard);
		}
	}

	if(CardVec.size() != MAX_BATTLE_NUM)
		return OPERATOR_BOSS_BATTLE_NUM;

	BATTLE_MAP& BattleMap = m_BossBattleMap[nID];
	BossBattleInfoRef pBattle = BossBattleInfoRef(new stBossBattleInfo);
	pBattle->BossId = nID;
	pBattle->PlayerId = nPlayerID;
	pBattle->CoolDown = time(NULL);
	for (auto i = 0; i < MAX_BATTLE_NUM; ++i)
	{
		pBattle->m_Card[i] = CardVec[i];
	}

	BattleMap.insert(BATTLE_MAP::value_type(nPlayerID, pBattle));
	stBattleInfoDB pInfo;
	pInfo = pBattle.get();
	ANSYDB(0, BOSSBATTLEDB.InsertSql(&pInfo));
	return OPERATOR_NONE_ERROR;
}

void CBossMgr::AttackBoss(BossRef pData, BossBattleInfoRef pBattle) {
	F32 fDamage = 0;
	S32 nDamage = 0;
	if (pData) {
		F32 fVal[MAX_BATTLE_NUM] = { 0.0f, 0.0f, 0.0f };
		stCardBattleInfo& pCardInfo = pBattle->m_Card[pBattle->CurPos];
		if (pData->Type == 0) {
			fVal[0] = 1.0f;
			fVal[1] = 0.5f;
			fVal[2] = 2.0f;
		}
		else if (pData->Type == 1) {
			fVal[0] = 2.0f;
			fVal[1] = 1.0f;
			fVal[2] = 0.5f;
		}
		else {
			fVal[0] = 0.5f;
			fVal[1] = 2.0f;
			fVal[2] = 1.0f;
		}
	

		fDamage = (pCardInfo.Agi * (1.0f + (pCardInfo.Agi_Grow / 100.0f))) * fVal[0] + \
			(pCardInfo.Int * (1.0f + (pCardInfo.Int_Grow / 100.0f))) * fVal[1] + \
			(pCardInfo.Str * (1.0f + (pCardInfo.Str_Grow / 100.0f))) * fVal[2];

		fDamage *= gRandGen.randF(0.90f, 1.10f);
		fDamage *= (1.0f + (pBattle->m_Buff[0].Add_gPc / 100.0f) + (pBattle->m_Buff[1].Add_gPc / 100.0f));
		nDamage = fDamage;
		if (nDamage > 0) {
			pData->Hp -= nDamage;
			ANSYDB(0, BOSSDB.UpdateSqlEx(pData, "Hp"));
			auto itr = m_BossBattleRankMap[pBattle->BossId].find(pBattle->PlayerId);
			if (itr != m_BossBattleRankMap[pBattle->BossId].end()) {
				itr->second->Hp += nDamage;
				ANSYDB(0, BOSSRANKDB.UpdateSqlEx(itr->second, "Hp"));
			}
			else {
				BossBattleRankRef pData = BossBattleRankRef(new stBattleRank);
				pData->BossId = pBattle->BossId;
				pData->PlayerId = pBattle->PlayerId;
				pData->Hp = nDamage;
				pData->PlayerName = SERVER->GetPlayerManager()->GetPlayerName(pData->PlayerId);
				m_BossBattleRankMap[pData->BossId].insert(BATTLE_RANK_MAP::value_type(pData->PlayerId, pData));
				ANSYDB(0, BOSSRANKDB.InsertSql(pData));
			}
		}
	}
}

enOpError CBossMgr::BuyBuff(S32 nPlayerID, S32 nID,  S32 Type) {
	auto itr = m_BossMap.find(nID);
	if (itr == m_BossMap.end()) {
		return OPERATOR_BOSS_NO;
	}

	BossBattleInfoRef pBattle;
	S32 nCurTime = time(NULL);
	auto itr1 = m_BossBattleMap.find(nID);
	if (itr1 != m_BossBattleMap.end()) {
		auto itr2 = itr1->second.find(nPlayerID);
		if (itr2 == itr1->second.end()) {
			return OPERATOR_BOSS_PLAYER_NO;
		}

		pBattle = itr2->second;
	}

	Type = mClamp(Type, 0, 1);
	S32 nAddVal = 0;
	if (Type == 0 )
	{
		if (!PLAYERINFOMGR->CanAddValue(nPlayerID, Money, -100 * 10000))
			return OPERATOR_HASNO_MONEY;

		if (pBattle->m_Buff[0].CoolDown >= (nCurTime + 90))
			return OPERATOR_PARAMETER_ERROR;

		nAddVal = 15;
	}
	else {
		if (!SERVER->GetPlayerManager()->CanAddGold(nPlayerID, -50))
			return OPERATOR_HASNO_GOLD;

		nAddVal = 50;
	}
	pBattle->m_Buff[Type].Add_gPc = nAddVal;
	if (pBattle->m_Buff[Type].CoolDown < nCurTime)
		pBattle->m_Buff[Type].CoolDown = nCurTime + 3 * 60;
	else
		pBattle->m_Buff[Type].CoolDown += 3 * 60;

	stBattleInfoDB pInfo;
	pInfo = pBattle.get();
	ANSYDB(0, BOSSBATTLEDB.UpdateSqlEx(&pInfo, "Add_gPc", "CoolDown"));
	return OPERATOR_NONE_ERROR;
}

void CBossMgr::TimeProcess(U32 bExit) {
	S32 nCurTime = time(NULL);
	for (auto itr = m_BossMap.begin(); itr != m_BossMap.end();) {
		if (!itr->second) {
			itr = m_BossMap.erase(itr);
		}

		BATTLE_MAP& BattleMap = m_BossBattleMap[itr->first];
		for (auto iter = BattleMap.begin(); iter != BattleMap.end(); ++iter) {
			BossBattleInfoRef pBattle = iter->second;
			if (pBattle && pBattle->CoolDown <= nCurTime)
			{
				AttackBoss(itr->second, pBattle);
				pBattle->CurPos = (pBattle->CurPos + 1) % MAX_BATTLE_NUM;
				pBattle->CoolDown = nCurTime + 10;
				stBattleInfoDB pInfo;
				pInfo = pBattle.get();
				ANSYDB(0, BOSSBATTLEDB.UpdateSqlEx(&pInfo, "CurPos", "CoolDown"));
			}

			//清空buff
			if (pBattle)
			{
				for (auto i = 0; i < MAX_BUFF_NUM; ++i) {
					bool bEnd = false;
					if (pBattle->m_Buff[0].CoolDown <= (nCurTime + 90) && pBattle->m_Buff[0].Add_gPc != 0) {
						pBattle->m_Buff[0].Add_gPc = 0;
						bEnd = true;
					}
					if (pBattle->m_Buff[1].CoolDown <= nCurTime  && pBattle->m_Buff[1].Add_gPc != 0) {
						pBattle->m_Buff[1].Add_gPc = 0;
						bEnd = true;
					}

					if (bEnd)
					{
						stBattleInfoDB pInfo;
						pInfo = pBattle.get();
						ANSYDB(0, BOSSBATTLEDB.UpdateSqlEx(&pInfo, "Add_gPc"));
					}
				}
			}
		}

		//boss被击败
		if (itr->second->Hp <= 0 || itr->second->EndTime <= nCurTime)
		{
			auto pData = itr->second;
			auto Reward = [&]() {
				std::string Title = "联盟副本战报";
				auto pBossData = g_BossDataMgr->getData(pData->DataId);
				if (pBossData)
				{
					bool bDefend = itr->second->Hp <= 0;
					auto pOrg = ORGMGR->GetOrg(pData->BossId);
					if (bDefend) {
						for (auto i = 0; i < 4; ++i) {
							pOrg->AddGift(pBossData->m_AtkItem[i], pBossData->m_Name);
						}
					}

					S32 nVal = 0;
					for (auto itr1 = m_BossBattleRankMap[pData->BossId].begin(); itr1 != m_BossBattleRankMap[pData->BossId].end(); ++itr1) {
						if (pOrg) {
							nVal = itr1->second->Hp / (1.0f * pBossData->m_Hp) * pBossData->m_Award + pBossData->m_Award1;
							pOrg->AddContribute(itr1->second->PlayerId, nVal);
						}

						PLAYERINFOMGR->AddMoney(itr1->second->PlayerId, pBossData->m_Money);
						PLAYERINFOMGR->AddOre(itr1->second->PlayerId, pBossData->m_Ore);
						PLAYERINFOMGR->AddTroops(itr1->second->PlayerId, pBossData->m_Troops);

						{	
							F32 DamgePer = itr1->second->Hp / (1.0f * pBossData->m_Hp) * 100;
							std::ostringstream Content;
							Content << "联盟副本已结束，副本首领"
								<< (bDefend == true ? "已" : "未")
								<< "被击杀， 你本次对首领造成了"
								<< (S32)DamgePer
								<< "%的伤害,获得"
								<< nVal
								<< "联盟贡献、"
								<< pBossData->m_Money
								<< "银两、"
								<< pBossData->m_Ore
								<< "矿石、"
								<< pBossData->m_Troops
								<< "兵力奖励";
							//std::string Content = fmt::sprintf("联盟副本已结束，副本首领%s被击杀， 你本次对首领造成了%.2f的伤害，获得%d联盟贡献、%d银两、%d矿石、%d兵力奖励", bDefend == true ? "已" : "未", DamgePer, nVal, pBossData->m_Money, pBossData->m_Ore, pBossData->m_Troops);
							MAILMGR->sendMail(0, itr1->second->PlayerId, Title.c_str(), Content.str().c_str(), true);
						}
					}
				}			
				
				//清楚数据
				ANSYDB(0, fmt::sprintf("DELETE FROM %s WHERE BossId=%d", BOSSBATTLETABLE, itr->first));
				ANSYDB(0, fmt::sprintf("DELETE FROM %s WHERE BossId=%d", BOSSRANKTABLE, itr->first));
				ANSYDB(0, fmt::sprintf("DELETE FROM %s WHERE BossId=%d", BOSSTABLE, itr->first));
				m_BossBattleMap[pData->BossId].clear();
				m_BossBattleMap.erase(pData->BossId);
				m_BossBattleRankMap[pData->BossId].clear();
				m_BossBattleRankMap.erase(pData->BossId);
				itr = m_BossMap.erase(itr);
			};

			Reward();
		}
		else {
			++itr;
		}
	}
}

void CBossMgr::SendInitToClient(U32 nPlayerID)
{
	AccountRef pAccount = SERVER->GetPlayerManager()->GetOnlinePlayer(nPlayerID);
	if (pAccount)
	{
		CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_INIT_TECHNOLOGY_NOTIFY", pAccount->GetAccountId(), SERVICE_CLIENT);

		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}
}

void CBossMgr::LoadDB()
{
	{
		TBLExecution tHandle(SERVER->GetActorDB());;
		tHandle.SetId(0);
		tHandle.SetSql(BOSSDB.LoadSql());
		tHandle.RegisterFunction([this](int id, int error, void* pSqlHandle)
		{
			CDBConn * pHandle = (CDBConn*)(pSqlHandle);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					BossRef pData = BossRef(new stBoss);
					pData->BossId = pHandle->GetInt();
					pData->DataId = pHandle->GetInt();
					pData->Hp = pHandle->GetBigInt();
					pData->Type = pHandle->GetInt();
					pData->CreateTime = pHandle->GetTime();
					pData->EndTime = pHandle->GetTime();
					AddData(pData);
				}
			}
		}
		);
		tHandle.Commint();
	}

	{
		TBLExecution tHandle(SERVER->GetActorDB());;
		tHandle.SetId(0);
		tHandle.SetSql(BOSSRANKDB.LoadSql());
		tHandle.RegisterFunction([this](int id, int error, void* pSqlHandle)
		{
			CDBConn * pHandle = (CDBConn*)(pSqlHandle);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					BossBattleRankRef pData = BossBattleRankRef(new stBattleRank);
					pData->BossId = pHandle->GetInt();
					pData->PlayerId = pHandle->GetInt();
					pData->Hp = pHandle->GetBigInt();
					pData->PlayerName = SERVER->GetPlayerManager()->GetPlayerName(pData->PlayerId);
					m_BossBattleRankMap[pData->BossId].insert(BATTLE_RANK_MAP::value_type(pData->PlayerId, pData));
				}
			}
		}
		);
		tHandle.Commint();
	}

	{
		TBLExecution tHandle(SERVER->GetActorDB());;
		tHandle.SetId(0);
		tHandle.SetSql(BOSSBATTLEDB.LoadSql());
		tHandle.RegisterFunction([this](int id, int error, void* pSqlHandle)
		{
			CDBConn * pHandle = (CDBConn*)(pSqlHandle);
			if (pHandle && error == NONE_ERROR)
			{
				while (pHandle->More())
				{
					BossBattleInfoRef pData = BossBattleInfoRef(new stBossBattleInfo);
					pData->BossId = pHandle->GetInt();
					pData->PlayerId = pHandle->GetInt();
					pData->CurPos = pHandle->GetInt();
					for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
						pData->m_Card[i].UID = pHandle->GetString();
					}
					for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
						pData->m_Card[i].Agi = pHandle->GetInt();
					}
					for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
						pData->m_Card[i].Agi_Grow = pHandle->GetInt();
					}
					for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
						pData->m_Card[i].Int = pHandle->GetInt();
					}
					for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
						pData->m_Card[i].Int_Grow = pHandle->GetInt();
					}
					for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
						pData->m_Card[i].Str = pHandle->GetInt();
					}
					for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
						pData->m_Card[i].Str_Grow = pHandle->GetInt();
					}
					for (auto i = 0; i < MAX_BUFF_NUM; ++i) {
						pData->m_Buff[i].Add_gPc = pHandle->GetInt();
					}
					for (auto i = 0; i < MAX_BUFF_NUM; ++i) {
						pData->m_Buff[i].CoolDown = pHandle->GetTime();
					}

					pData->CoolDown = pHandle->GetTime();
					m_BossBattleMap[pData->BossId].insert(BATTLE_MAP::value_type(pData->PlayerId, pData));
				}
			}
		}
		);
		tHandle.Commint();
	}
}

bool CBossMgr::UpdateDB(BossRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(BOSSDB.UpdateSql(pData));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CBossMgr::InsertDB(BossRef pData)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(BOSSDB.InsertSql(pData));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}

bool CBossMgr::DeleteDB(S32 ID)
{
	DB_Execution* pHandle = new DB_Execution();
	pHandle->SetSql(fmt::sprintf("DELETE FROM %s WHERE BossID=%d", BOSSTABLE, ID));
	SERVER->GetPlayerDBManager()->SendPacket(pHandle);
	return true;
}


void CBossMgr::Export(struct lua_State* L)
{

}

bool CBossMgr::HandleBossOpenOrgResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 OrgId = Packet->readInt(Base::Bit32);
	S32 DataId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		do{
			auto pOrg = ORGMGR->GetOrg(OrgId);
			if (!pOrg)
			{
				nError = OPERATOR_ORG_NO;
				break;
			}

			if (!pOrg->IsMasterId(pAccount->GetPlayerId()) && !pOrg->IsSubMasterId(pAccount->GetPlayerId()))
			{
				nError = OPERATOR_ORG_MASTER_NOT;
				break;
			}

			if (!pOrg->CanOpenBoss()) {
				nError = OPERATOR_ORG_BOSS_MAX;
				break;
			}

			nError = CreateBoss(OrgId, DataId);
			if (nError == OPERATOR_NONE_ERROR) {
				pOrg->SetBossTimes();
			}
		}while (false);
	}

	{
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_Boss_OpenOrg_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return true;
}

bool CBossMgr::HandleBossBuyBuffResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 BossId = Packet->readInt(Base::Bit32);
	S32 Type = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = BuyBuff(pAccount->GetPlayerId(), BossId, Type);
	}

	{
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_Boss_BuyBuff_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return true;
}

bool CBossMgr::HandleBossInfoResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) {
	U32 nAccountId = pHead->Id;
	S32 BossId = Packet->readInt(Base::Bit32);
	S32 nError = 0;
	bool bBattle = false;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		stBuffInfo buff[2];
		auto pBoss = GetData(BossId);
		if(pBoss)
		{
			BossBattleInfoRef pBattle;
			auto itr = m_BossBattleMap[BossId].find(pAccount->GetPlayerId());
			if (itr != m_BossBattleMap[BossId].end()) {
				if (itr->second)
				{
					pBattle = itr->second;
					bBattle = true;
					for (auto i = 0; i < MAX_BUFF_NUM; ++i)
					{
						buff[i] = itr->second->m_Buff[i];
					}
				}
			}

			auto& BattleRankMap = m_BossBattleRankMap[BossId];
			CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_BossInfo_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
			sendPacket.writeInt(0, Base::Bit16);
			sendPacket.writeInt(pBoss->DataId, Base::Bit32);
			sendPacket.writeFlag(bBattle);
			sendPacket.writeInt(buff[0].CoolDown, Base::Bit32);
			sendPacket.writeInt(buff[1].CoolDown, Base::Bit32);
			//BOSS
			sendPacket.writeInt(1, Base::Bit16);
			for (auto i = 0; i < 1; ++i) {
				if (sendPacket.writeFlag(pBoss != NULL)) {
					sendPacket.writeInt(pBoss->Type, Base::Bit32);
					sendPacket.writeInt(pBoss->EndTime, Base::Bit32);
					sendPacket.writeBits(sizeof(pBoss->Hp) << 3, &pBoss->Hp);
				}
			}
			sendPacket.writeInt(BattleRankMap.size(), Base::Bit16);
			for (auto itr = BattleRankMap.begin(); itr != BattleRankMap.end(); ++itr) {
				if (sendPacket.writeFlag(itr->second != NULL)) {
					sendPacket.writeString(Util::MbcsToUtf8(itr->second->PlayerName));
					sendPacket.writeBits(sizeof(itr->second->Hp) << 3, &itr->second->Hp);
				}
			}

			//大臣
			sendPacket.writeInt(3, Base::Bit16);
			for (auto i = 0; i < 3; ++i) {
				if (sendPacket.writeFlag(pBattle != NULL)) {
					sendPacket.writeString(pBattle->m_Card[i].UID);
				}
			}

			//BOSS
			sendPacket.writeInt(1, Base::Bit16);
			for (auto i = 0; i < 1; ++i) {
				if (sendPacket.writeFlag(pBattle != NULL)) {
					sendPacket.writeInt(pBattle->CurPos, Base::Bit32);
					sendPacket.writeInt(pBattle->CoolDown, Base::Bit32);
				}
			}
			pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}
		else {
			CMemGuard Buffer(MAX_PACKET_SIZE MEM_GUARD_PARAM);
			Base::BitStream sendPacket(Buffer.get(), MAX_PACKET_SIZE);
			stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_BossInfo_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
			sendPacket.writeInt(0, Base::Bit16);
			sendPacket.writeInt(0, Base::Bit32);
			sendPacket.writeFlag(bBattle);
			sendPacket.writeInt(buff[0].CoolDown, Base::Bit32);
			sendPacket.writeInt(buff[1].CoolDown, Base::Bit32);
			sendPacket.writeInt(0, Base::Bit16);
			sendPacket.writeInt(0, Base::Bit16);
			sendPacket.writeInt(0, Base::Bit16);
			sendPacket.writeInt(0, Base::Bit16);
			pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
			SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
		}
	}

	return true;
}

bool CBossMgr::HandleBossBattleResponse(int SocketHandle, stPacketHead* pHead, Base::BitStream *Packet) 
{
	U32 nAccountId = pHead->Id;
	S32 BossId = Packet->readInt(Base::Bit32);
	std::string UID[3] = { Packet->readString(),  Packet->readString(), Packet->readString() };
	S32 nError = 0;
	AccountRef pAccount = SERVER->GetPlayerManager()->GetAccount(nAccountId);
	if (pAccount)
	{
		nError = BattleBoss(pAccount->GetPlayerId(), BossId, UID);
	}

	{
		CMemGuard Buffer(128 MEM_GUARD_PARAM);
		Base::BitStream sendPacket(Buffer.get(), 128);
		stPacketHead* pHead = IPacket::BuildPacketHead(sendPacket, "WC_Boss_Battle_Response", pAccount->GetAccountId(), SERVICE_CLIENT);
		sendPacket.writeInt(nError, Base::Bit16);
		pHead->PacketSize = sendPacket.getPosition() - sizeof(stPacketHead);
		SERVER->GetServerSocket()->Send(pAccount->GetGateSocketId(), sendPacket);
	}

	return true;
}