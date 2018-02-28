/////////////////////////////////////////////////////////////////
/// @file OrgBase.h
///
/// 家族公共文件.
///
/// 家族公共文件, 用于定义公共结构体等.
///
///	@作者 LivenHotch.
///
/// @日期  06/03/2010
//  -------------------------------------------------------------
//  修改记录.
//  06/03/2010    文件创建
//
//  Copyright (C) 2010 - All Rights Reserved
/////////////////////////////////////////////////////////////////

#ifndef ORG_BASE_H
#define ORG_BASE_H

#include "base/log.h"
#if defined(NTJ_SERVER) || defined(NTJ_CLIENT) || defined(NTJ_EDITOR)
#include "platform/types.h"
#include "platform/platformAssert.h"
#else
#include "base/types.h"
#endif
#include "PacketType.h"
#include <memory.h>
#include <assert.h>
#include "PlayerStruct.h"

#define HasFlag(x, y) !((x) & (y) ^ (y))
#define ClrFlag(x, y) (x) &= (~(y))
#define HasAndClr(x, y) HasFlag(x, y) && (ClrFlag(x, y), true)

inline bool If_condition(bool result, const char* exp, const char* file, const char* func, int line)
{
	if (result)
	{
		const char* fileName = strrchr(file, '\\');
		if (fileName)
			fileName = fileName + 1;
		else
			fileName = file;
		char buffer[1024];
		sprintf_s(buffer, sizeof(buffer), "illegal condition[%s:%s:%d]: %s.", fileName, func, line, exp);
#ifdef _DEBUG
#ifndef _NTJ_UNITTEST
		assert(!buffer);
#endif
#endif
		g_Log.WriteError(buffer);
	}
	return result;
}
#define IF_ASSERT(x) if (If_condition(x, #x, __FILE__, __FUNCTION__, __LINE__))
#define JUST_ASSERT(x) If_condition(x, #x, __FILE__, __FUNCTION__, __LINE__)
#define IF_NOT_ASSERT(x) if (!If_condition(!(x), #x, __FILE__, __FUNCTION__, __LINE__))
#define DOASSERT(x, info) If_condition(!(x), info, __FILE__, __FUNCTION__, __LINE__)

#define ORGS_PER_PAGE 16
#define DEPOTS_PER_PAGE 9
#define ORGMONEY_CONTRIBUTE_RATIO 1
#define ORG_DB_INTERVAL 3*60
#define ORG_CUMULATE_ITEM 105102633
#define ORG_NIMBUS_ITER 1
#define ORG_TRADE_NUM 100
#define ORG_TRADE_LEVEL 2
#define ORG_CREATE_MONEY 10*10000
#define LEAGUE_CREATE_MONEY 100*10000
#define LEAGUE_APPLY_UPDATE 3*60
#define LEAGUE_SEARCH_ROWS 16
#define LEAGUES_PER_TIME 8
#define ORG_TRANSFER_INTERVAL 2*24*60*60
#define LEAGUE_TRANSFER_INTERVAL 7*24*60*60
#define ORG_ASSAULT_TIME 4*60*60
#define ORG_ASSAULT_MONEY 1000
#define ORG_MEMBER_UPDATE_INTERVAL 10*60
#define ORG_ACTIVITY_UPDATE_INTERVAL 10*60*1000
#define ORG_UPDATE_INTERVAL 3*60
#define ORG_DEMESNE_APPLY_ORGMONEY 2000
#define ORG_DEMESNE_APPLY_MONEY 200000
#define LEAGUE_ORGDEMESNE_LIMIT 2
#define ORG_STATISTICS_INTERVAL 10*60
#define LEAGUE_DEMESNE_INCOME 10000000
#define LEAGUE_DEMESNE_NIMBUS 3000
#define LEAGUE_DEMESNE_APPLY_MONEY 1000000
#define ORG_SETSUBMASTER_INTERVAL 2*24*60*60
#define ORG_MAX_EFFGYNAME 80
#define ORG_TAX_ITEM 106069901
#define ORG_TAX_ITEM_VALUE 10000
#define ORG_MAX_TAX_ITEM 76
#define ORG_MAX_INCOME 7000000
#define ORG_DEMESNE_LEVEL 3
#define LEAGUE_DEMESNE_LEVEL 2
#define ORG_ASSAULT_LEVEL 2
#define ORG_JOIN_TIME_LIMIT 3*24*60*60
#define LEAGUE_JOIN_TIME_LIMIT 3*24*60*60
#define MAX_APPLY_NUM 100

#define MIN_ORG_PACKET sizeof(Org)/*+sizeof(OrgMaster)*/+sizeof(OrgRes)+40
#define MID_ORG_PACKET sizeof(Org)/*+sizeof(OrgMaster)*/+sizeof(OrgRes)+sizeof(OrgMember)*Org::GetMemLimit(Org::_maxLevel)+10
#define FULL_ORG_PACKET MID_ORG_PACKET+sizeof(OrgAssaultStat)+sizeof(OrgDepot)*Org::_maxDepot+sizeof(OrgBoss)*Org::_maxBoss+10
#define QUERY_ORG_PACKET (MIN_ORG_PACKET)*ORGS_PER_PAGE+sizeof(stPacketHead)+10
#define NORMAL_ORG_PACKET FULL_ORG_PACKET+sizeof(OrgAssault)+(League::_maxOrg-1)*(MID_ORG_PACKET)+sizeof(stPacketHead)+10
#define QUERY_LEAGUE_PACKET LEAGUES_PER_TIME*League::_maxOrg*(MIN_ORG_PACKET)+sizeof(stPacketHead)+10

#define MAX_ORG_PACKET std::max(std::max(QUERY_ORG_PACKET, NORMAL_ORG_PACKET), QUERY_LEAGUE_PACKET)
#define ORGSTR(x) #x
#define ORG_TRADE_COUNT(x) (Org::GetMemLimit(x)/2)
#define LEAGUE_HONOR(x) (x/100)
#define WORSHIP_CUMULATE(x) (x/100)

enum OrgOp
{
	ORGOP_NULL,

	ORG_CreateRequest,
	ORG_SetName,
	ORG_Create,
	ORG_Disband,
	ORG_Announce,
	ORG_TransferLeader,
	ORG_SetPassword,
	ORG_GetApply,
	ORG_AutoAcceptJoin,
	ORG_SetYYChannel,
	ORG_CancelDisband,
	ORG_Assault,
	ORG_SetSubMaster,
	ORG_ApplyDemesne,
	ORG_OccupyDemesne,
	ORG_ChangeTradeRight,

	ORG_ApplyJoin,
	ORG_AddRequest,
	ORG_AddMember,
	ORG_ModifyRemark,
	ORG_RemoveMember,
	ORG_Leave,
	ORG_AcceptJoin,
	ORG_QuerySalary,
	ORG_GetPay,
	ORG_ReplyDisband,

	ORG_Study,
	ORG_Manufacture,
	ORG_Buy,
	ORG_AutoMake,
	ORG_UndoBuy,

	ORG_Shopping,
	ORG_UndoShopping,

	ORG_SetCumulate,
	ORG_Cumulate,
	ORG_CallBoss,

	ORG_AddMoney,
	ORG_AddHonor,
	ORG_AddLevel,
	ORG_AddActivity,
	ORG_DelTradeCount,
	ORG_BossCumulate,
	ORG_ClearBossNimbus,
	ORG_AddBossNimbus,
	ORG_AddMemberMoney,
	ORG_AddMemberLeagueMoney,
	ORG_AddMemberContribute,
	ORG_AddBossCall,
	ORG_AddBossKill,
	ORG_Kill,

	LEAGUE_CreateRequest,
	LEAGUE_SetName,
	LEAGUE_Create,
	LEAGUE_Announce,
	LEAGUE_TransferLeader,
	LEAGUE_TransferSubLeader,
	LEAGUE_GetApply,
	LEAGUE_AutoAcceptJoin,
	LEAGUE_Disband,
	LEAGUE_SetYYChannel,
	LEAGUE_ApplyDemesne,
	LEAGUE_OccupyDemesne,

	LEAGUE_AddOrg,
	LEAGUE_RemoveOrg,
	LEAGUE_AddRequest,
	LEAGUE_Leave,
	LEAGUE_AcceptJoin,
	LEAGUE_ApplyJoin,
	LEAGUE_ModifyRemark,

	LEAGUE_AddStone,
	LEAGUE_AddSpirite,
	LEAGUE_AddWood,
	LEAGUE_AddMoney,

	ORG_Result,

	ORG_Reply,
	ORG_Query,
	ORG_QueryOrg,
	ORG_AcceptMission,
	ORG_PutOutMission,
	ORG_CloseCreateWnd,

	ORG_Info,
	ORG_MemberInfo,
	ORG_ApplyInfo,
	ORG_AssaultInfo,
	ORG_DemesneInfo,
	ORG_QueryDemesne,

	ORG_UpdateMember,
	ORG_UpdateOrg,
	ORG_UpdateApply,
	ORG_UpdateDepot,
	ORG_UpdateRes,
	ORG_UpdateBoss,
	ORG_UpdateAssaultStat,
	ORG_UpdateDemesne,
	ORG_ResetDemesne,
	ORG_UpdateIncome,
	ORG_ResetIncome,

	LEAGUE_Reply,
	LEAGUE_CloseCreateWnd,
	LEAGUE_Query,

	LEAGUE_Info,
	LEAGUE_OrgInfo,
	LEAGUE_ApplyInfo,
	LEAGUE_QueryDemesne,

	LEAGUE_UpdateApply,
	LEAGUE_UpdateLeague,
	LEAGUE_UpdateRes,
	LEAGUE_UpdateDemesne,
	LEAGUE_ResetDemesne,
	LEAGUE_UpdateIncome,
	LEAGUE_ResetIncome,

	ORG_SendMemberContributeRank,

	ORG_Notify,

	ORGOP_COUNT
};

static const char* strOrgOp[] =
{
	ORGSTR(ORGOP_NULL),

	ORGSTR(ORG_CreateRequest),
	ORGSTR(ORG_SetName),
	ORGSTR(ORG_Create),
	ORGSTR(ORG_Disband),
	ORGSTR(ORG_Announce),
	ORGSTR(ORG_TransferLeader),
	ORGSTR(ORG_SetPassword),
	ORGSTR(ORG_GetApply),
	ORGSTR(ORG_AutoAcceptJoin),
	ORGSTR(ORG_SetYYChannel),
	ORGSTR(ORG_CancelDisband),
	ORGSTR(ORG_Assault),
	ORGSTR(ORG_SetSubMaster),
	ORGSTR(ORG_ApplyDemesne),
	ORGSTR(ORG_OccupyDemesne),
	ORGSTR(ORG_ChangeTradeRight),

	ORGSTR(ORG_ApplyJoin),
	ORGSTR(ORG_AddRequest),
	ORGSTR(ORG_AddMember),
	ORGSTR(ORG_ModifyRemark),
	ORGSTR(ORG_RemoveMember),
	ORGSTR(ORG_Leave),
	ORGSTR(ORG_AcceptJoin),
	ORGSTR(ORG_QuerySalary),
	ORGSTR(ORG_GetPay),
	ORGSTR(ORG_ReplyDisband),

	ORGSTR(ORG_Study),
	ORGSTR(ORG_Manufacture),
	ORGSTR(ORG_Buy),
	ORGSTR(ORG_AutoMake),
	ORGSTR(ORG_UndoBuy),

	ORGSTR(ORG_Shopping),
	ORGSTR(ORG_UndoShopping),

	ORGSTR(ORG_SetCumulate),
	ORGSTR(ORG_Cumulate),
	ORGSTR(ORG_CallBoss),

	ORGSTR(ORG_AddMoney),
	ORGSTR(ORG_AddHonor),
	ORGSTR(ORG_AddLevel),
	ORGSTR(ORG_AddActivity),
	ORGSTR(ORG_DelTradeCount),
	ORGSTR(ORG_BossCumulate),
	ORGSTR(ORG_ClearBossNimbus),
	ORGSTR(ORG_AddBossNimbus),
	ORGSTR(ORG_AddMemberMoney),
	ORGSTR(ORG_AddMemberLeagueMoney),
	ORGSTR(ORG_AddMemberContribute),
	ORGSTR(ORG_AddBossCall),
	ORGSTR(ORG_AddBossKill),
	ORGSTR(ORG_Kill),

	ORGSTR(LEAGUE_CreateRequest),
	ORGSTR(LEAGUE_SetName),
	ORGSTR(LEAGUE_Create),
	ORGSTR(LEAGUE_Announce),
	ORGSTR(LEAGUE_TransferLeader),
	ORGSTR(LEAGUE_TransferSubLeader),
	ORGSTR(LEAGUE_GetApply),
	ORGSTR(LEAGUE_AutoAcceptJoin),
	ORGSTR(LEAGUE_Disband),
	ORGSTR(LEAGUE_SetYYChannel),
	ORGSTR(LEAGUE_ApplyDemesne),
	ORGSTR(LEAGUE_OccupyDemesne),

	ORGSTR(LEAGUE_AddOrg),
	ORGSTR(LEAGUE_RemoveOrg),
	ORGSTR(LEAGUE_AddRequest),
	ORGSTR(LEAGUE_Leave),
	ORGSTR(LEAGUE_AcceptJoin),
	ORGSTR(LEAGUE_ApplyJoin),
	ORGSTR(LEAGUE_ModifyRemark),

	ORGSTR(LEAGUE_AddStone),
	ORGSTR(LEAGUE_AddSpirite),
	ORGSTR(LEAGUE_AddWood),
	ORGSTR(LEAGUE_AddMoney),

	ORGSTR(ORG_Result),

	ORGSTR(ORG_Reply),
	ORGSTR(ORG_Query),
	ORGSTR(ORG_QueryOrg),
	ORGSTR(ORG_AcceptMission),
	ORGSTR(ORG_PutOutMission),
	ORGSTR(ORG_CloseCreateWnd),

	ORGSTR(ORG_Info),
	ORGSTR(ORG_MemberInfo),
	ORGSTR(ORG_ApplyInfo),
	ORGSTR(ORG_AssaultInfo),
	ORGSTR(ORG_DemesneInfo),
	ORGSTR(ORG_QueryDemesne),

	ORGSTR(ORG_UpdateMember),
	ORGSTR(ORG_UpdateOrg),
	ORGSTR(ORG_UpdateApply),
	ORGSTR(ORG_UpdateDepot),
	ORGSTR(ORG_UpdateRes),
	ORGSTR(ORG_UpdateBoss),
	ORGSTR(ORG_UpdateAssaultStat),
	ORGSTR(ORG_UpdateDemesne),
	ORGSTR(ORG_ResetDemesne),
	ORGSTR(ORG_UpdateIncome),
	ORGSTR(ORG_ResetIncome),

	ORGSTR(LEAGUE_Reply),
	ORGSTR(LEAGUE_CloseCreateWnd),
	ORGSTR(LEAGUE_Query),

	ORGSTR(LEAGUE_Info),
	ORGSTR(LEAGUE_OrgInfo),
	ORGSTR(LEAGUE_ApplyInfo),
	ORGSTR(LEAGUE_QueryDemesne),

	ORGSTR(LEAGUE_UpdateApply),
	ORGSTR(LEAGUE_UpdateLeague),
	ORGSTR(LEAGUE_UpdateRes),
	ORGSTR(LEAGUE_UpdateDemesne),
	ORGSTR(LEAGUE_ResetDemesne),
	ORGSTR(LEAGUE_UpdateIncome),
	ORGSTR(LEAGUE_ResetIncome),

	ORGSTR(ORG_SendMemberContributeRank),

	ORGSTR(ORG_Notify)
};

struct OrgMaster
{
	char	realName[COMMON_STRING_LENGTH];
	char	telphone[COMMON_STRING_LENGTH];
	char	idCode[COMMON_STRING_LENGTH];
	char	email[COMMON_STRING_LENGTH];
	int		city;

	OrgMaster()
	{
		memset(this, 0, sizeof(OrgMaster));
		dStrcpy(realName, sizeof(realName), "");
		dStrcpy(telphone, sizeof(telphone), "");
		dStrcpy(idCode, sizeof(idCode), "");
		dStrcpy(email, sizeof(email), "");
	}

	OrgMaster(const OrgMaster& other)
	{
		*this = other;
	}

	OrgMaster& operator=(const OrgMaster& other)
	{
		memcpy(this, &other, sizeof(OrgMaster));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeString(realName, _countof(realName));
		packet->writeString(telphone, _countof(telphone));
		packet->writeString(idCode, _countof(idCode));
		packet->writeString(email, _countof(email));
		packet->writeInt(city, Base::Bit16);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readString(realName, _countof(realName));
		packet->readString(telphone, _countof(telphone));
		packet->readString(idCode, _countof(idCode));
		packet->readString(email, _countof(email));
		city = packet->readInt(Base::Bit16);
	}
};

struct OrgMemberStat
{
	int statOrgHonor;
	int statOrgMoney;
	int statContribute;
	int statCumulate;
	int statTrade;
	int statMission;
	int statWeekOrgHonor;
	int statWeekOrgMoney;
	int statWeekContribute;
	int statWeekCumulate;
	int statWeekTrade;
	int statWeekMission;
	int statLastWeekOrgHonor;
	int statLastWeekOrgMoney;
	int statLastWeekContribute;
	int statLastWeekCumulate;
	int statLastWeekTrade;
	int statLastWeekMission;

	OrgMemberStat()
	{
		memset(this, 0, sizeof(OrgMemberStat));
	}

	OrgMemberStat(const OrgMemberStat& other)
	{
		*this = other;
	}

	OrgMemberStat& operator=(const OrgMemberStat& other)
	{
		memcpy(this, &other, sizeof(OrgMemberStat));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(OrgMemberStat) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(OrgMemberStat) << 3, this);
	}
};

struct OrgMember
{
	enum MEM_CHANGE
	{
		MEM_CHG_NULL = 0,
		MEM_CHG_Level = BIT(0),
		MEM_CHG_Contribute = BIT(1),
		MEM_CHG_Memo = BIT(2),
		MEM_CHG_OfflineTime = BIT(3),
		MEM_CHG_Money = BIT(4),
		MEM_CHG_AwardMoney = BIT(5),
		MEM_CHG_LeagueMoney = BIT(6),
		MEM_CHG_DisbandReply = BIT(7),
		MEM_CHG_KillCount = BIT(8),
		MEM_CHG_SubMaster = BIT(9),
		MEM_CHG_TradeRight = BIT(10),
		MEM_CHG_TodayTrade = BIT(11),
		MEM_CHG_Stat = BIT(12),
		MEM_CHG_StatWeek = BIT(13),
		MEM_CHG_StatLastWeek = BIT(14),
		MEM_CHG_Family = BIT(15),
	};

	int  playerId;
	char szPlayerName[COMMON_STRING_LENGTH];
	U8	 sex;
	U8	 level;
	U8	 nFamily;
	U8	 firstClass;
	int  contribute;
	char memo[COMMON_STRING_LENGTH];
	int	 offlineTime;
	int	 money;
	int	 awardMoney;
	int	 awardCanTradeMoney;
	int  leagueMoney;
	int	 disbandReply;
	int  CurKillCount;
	int  TotalKillCount;
	int	 lastUpdateTime;
	bool subMaster;
	bool tradeRight;
	int  todayTrade;
	OrgMemberStat statistics;

	OrgMember()
	{
		memset(this, 0, sizeof(OrgMember));
		dStrcpy(szPlayerName, sizeof(szPlayerName), "");
		dStrcpy(memo, sizeof(memo), "");
	}

	OrgMember(const OrgMember& other)
	{
		*this = other;
	}

	OrgMember& operator=(const OrgMember& other)
	{
		memcpy(this, &other, sizeof(OrgMember));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet, bool packStat) const
	{
		packet->writeInt(playerId, Base::Bit32);
		packet->writeInt(level, Base::Bit8);
		packet->writeInt(contribute, Base::Bit32);
		packet->writeString(memo, _countof(memo));
		packet->writeString(szPlayerName, _countof(szPlayerName));
		packet->writeInt(nFamily, Base::Bit8);
		packet->writeInt(offlineTime, Base::Bit32);
		packet->writeInt(sex, Base::Bit8);
		packet->writeInt(firstClass, Base::Bit8);
		packet->writeInt(money, Base::Bit32);
		packet->writeInt(awardMoney, Base::Bit32);
		packet->writeInt(awardCanTradeMoney, Base::Bit32);
		packet->writeInt(leagueMoney, Base::Bit32);
		packet->writeInt(disbandReply, Base::Bit8);
		packet->writeInt(CurKillCount, Base::Bit16);
		packet->writeInt(TotalKillCount, Base::Bit16);
		packet->writeFlag(subMaster);
		packet->writeFlag(tradeRight);
		packet->writeInt(todayTrade, Base::Bit16);
		if (packStat)
			statistics.WriteData(packet);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet, bool packStat)
	{
		playerId = packet->readInt(Base::Bit32);
		level = packet->readInt(Base::Bit8);
		contribute = packet->readInt(Base::Bit32);
		packet->readString(memo, _countof(memo));
		packet->readString(szPlayerName, _countof(szPlayerName));
		nFamily = packet->readInt(Base::Bit8);
		offlineTime = packet->readInt(Base::Bit32);
		sex = packet->readInt(Base::Bit8);
		firstClass = packet->readInt(Base::Bit8);
		money = packet->readInt(Base::Bit32);
		awardMoney = packet->readInt(Base::Bit32);
		awardCanTradeMoney = packet->readInt(Base::Bit32);
		leagueMoney = packet->readInt(Base::Bit32);
		disbandReply = packet->readInt(Base::Bit8);
		CurKillCount = packet->readInt(Base::Bit16);
		TotalKillCount = packet->readInt(Base::Bit16);
		subMaster = packet->readFlag();
		tradeRight = packet->readFlag();
		todayTrade = packet->readInt(Base::Bit16);
		if (packStat)
			statistics.ReadData(packet);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* packet) const
	{
		packet->writeInt(change, Base::Bit16);
		if (HasFlag(change, MEM_CHG_Level))
			packet->writeInt(level, Base::Bit8);
		if (HasFlag(change, MEM_CHG_Contribute))
			packet->writeInt(contribute, Base::Bit32);
		if (HasFlag(change, MEM_CHG_Memo))
			packet->writeString(memo, _countof(memo));
		if (HasFlag(change, MEM_CHG_OfflineTime))
			packet->writeInt(offlineTime, Base::Bit32);
		if (HasFlag(change, MEM_CHG_Money))
			packet->writeInt(money, Base::Bit32);
		if (HasFlag(change, MEM_CHG_AwardMoney))
		{
			packet->writeInt(money, Base::Bit32);
			packet->writeInt(awardMoney, Base::Bit32);
			packet->writeInt(awardCanTradeMoney, Base::Bit32);
		}
		if (HasFlag(change, MEM_CHG_LeagueMoney))
			packet->writeInt(leagueMoney, Base::Bit32);
		if (HasFlag(change, MEM_CHG_DisbandReply))
			packet->writeInt(disbandReply, Base::Bit8);
		if (HasFlag(change, MEM_CHG_KillCount))
		{
			packet->writeInt(CurKillCount, Base::Bit16);
			packet->writeInt(TotalKillCount, Base::Bit16);
		}
		if (HasFlag(change, MEM_CHG_SubMaster))
			packet->writeFlag(subMaster);
		if (HasFlag(change, MEM_CHG_TradeRight))
			packet->writeFlag(tradeRight);
		if (HasFlag(change, MEM_CHG_TodayTrade))
			packet->writeInt(todayTrade, Base::Bit16);
		if (HasFlag(change, MEM_CHG_StatWeek))
		{
			packet->writeInt(statistics.statWeekOrgHonor, Base::Bit32);
			packet->writeInt(statistics.statWeekOrgMoney, Base::Bit32);
			packet->writeInt(statistics.statWeekContribute, Base::Bit32);
			packet->writeInt(statistics.statWeekCumulate, Base::Bit32);
			packet->writeInt(statistics.statWeekTrade, Base::Bit32);
			packet->writeInt(statistics.statWeekMission, Base::Bit32);
		}
		if (HasFlag(change, MEM_CHG_Family))
			packet->writeInt(nFamily, Base::Bit8);
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* packet)
	{
		change = packet->readInt(Base::Bit16);
		if (HasFlag(change, MEM_CHG_Level))
			level = packet->readInt(Base::Bit8);
		if (HasFlag(change, MEM_CHG_Contribute))
			contribute = packet->readInt(Base::Bit32);
		if (HasFlag(change, MEM_CHG_Memo))
			packet->readString(memo, _countof(memo));
		if (HasFlag(change, MEM_CHG_OfflineTime))
			offlineTime = packet->readInt(Base::Bit32);
		if (HasFlag(change, MEM_CHG_Money))
			money = packet->readInt(Base::Bit32);
		if (HasFlag(change, MEM_CHG_AwardMoney))
		{
			money = packet->readInt(Base::Bit32);
			awardMoney = packet->readInt(Base::Bit32);
			awardCanTradeMoney = packet->readInt(Base::Bit32);
		}
		if (HasFlag(change, MEM_CHG_LeagueMoney))
			leagueMoney = packet->readInt(Base::Bit32);
		if (HasFlag(change, MEM_CHG_DisbandReply))
			disbandReply = packet->readInt(Base::Bit8);
		if (HasFlag(change, MEM_CHG_KillCount))
		{
			CurKillCount = packet->readInt(Base::Bit16);
			TotalKillCount = packet->readInt(Base::Bit16);
		}
		if (HasFlag(change, MEM_CHG_SubMaster))
			subMaster = packet->readFlag();
		if (HasFlag(change, MEM_CHG_TradeRight))
			tradeRight = packet->readFlag();
		if (HasFlag(change, MEM_CHG_TodayTrade))
			todayTrade = packet->readInt(Base::Bit16);
		if (HasFlag(change, MEM_CHG_StatWeek))
		{
			statistics.statWeekOrgHonor = packet->readInt(Base::Bit32);
			statistics.statWeekOrgMoney = packet->readInt(Base::Bit32);
			statistics.statWeekContribute = packet->readInt(Base::Bit32);
			statistics.statWeekCumulate = packet->readInt(Base::Bit32);
			statistics.statWeekTrade = packet->readInt(Base::Bit32);
			statistics.statWeekMission = packet->readInt(Base::Bit32);
		}
		if (HasFlag(change, MEM_CHG_Family))
			nFamily = packet->readInt(Base::Bit8);
	}
};

struct OrgRes
{
	enum RES_CHANGE
	{
		RES_CHG_NULL = 0,
		RES_CHG_Money = BIT(0),
		RES_CHG_Honor = BIT(1),
		RES_CHG_Activity = BIT(2),
		RES_CHG_MaxHonor = BIT(3),
		RES_CHG_MaxActivity = BIT(4),
		RES_CHG_TradeCount = BIT(5),
		RES_CHG_Order = BIT(6),
		RES_CHG_TopOrder = BIT(7)
	};
	int money;
	int stone;
	int honor;
	int sprite;
	int food;
	int drug;
	int wood;
	int cloth;
	int paper;
	int	activity;
	int maxHonor;
	int maxHonorTime;
	int maxActivity;
	int maxActivityTime;
	int tradeCount;
	int	order;
	int	topOrder;
	int topOrderTime;

	OrgRes()
	{
		memset(this, 0, sizeof(OrgRes));
		tradeCount = -1;
	}

	OrgRes(const OrgRes& other)
	{
		*this = other;
	}

	OrgRes& operator=(const OrgRes& other)
	{
		memcpy(this, &other, sizeof(OrgRes));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(money, Base::Bit32);
		packet->writeInt(honor, Base::Bit32);
		packet->writeInt(activity, Base::Bit32);
		packet->writeInt(maxHonor, Base::Bit32);
		packet->writeInt(maxHonorTime, Base::Bit32);
		packet->writeInt(maxActivity, Base::Bit32);
		packet->writeInt(maxActivityTime, Base::Bit32);
		packet->writeInt(tradeCount, Base::Bit32);
		packet->writeInt(order, Base::Bit16);
		packet->writeInt(topOrder, Base::Bit16);
		packet->writeInt(topOrderTime, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		money = packet->readInt(Base::Bit32);
		honor = packet->readInt(Base::Bit32);
		activity = packet->readInt(Base::Bit32);
		maxHonor = packet->readInt(Base::Bit32);
		maxHonorTime = packet->readInt(Base::Bit32);
		maxActivity = packet->readInt(Base::Bit32);
		maxActivityTime = packet->readInt(Base::Bit32);
		tradeCount = packet->readInt(Base::Bit32);
		order = packet->readInt(Base::Bit16);
		topOrder = packet->readInt(Base::Bit16);
		topOrderTime = packet->readInt(Base::Bit32);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* packet) const
	{
		packet->writeInt(change, Base::Bit8);
		if (HasFlag(change, RES_CHG_Money))
			packet->writeInt(money, Base::Bit32);
		if (HasFlag(change, RES_CHG_Honor))
			packet->writeInt(honor, Base::Bit32);
		if (HasFlag(change, RES_CHG_Activity))
			packet->writeInt(activity, Base::Bit32);
		if (HasFlag(change, RES_CHG_MaxHonor))
		{
			packet->writeInt(maxHonor, Base::Bit32);
			packet->writeInt(maxHonorTime, Base::Bit32);
		}
		if (HasFlag(change, RES_CHG_MaxActivity))
		{
			packet->writeInt(maxActivity, Base::Bit32);
			packet->writeInt(maxActivityTime, Base::Bit32);
		}
		if (HasFlag(change, RES_CHG_TradeCount))
			packet->writeInt(tradeCount, Base::Bit32);
		if (HasFlag(change, RES_CHG_Order))
			packet->writeInt(order, Base::Bit16);
		if (HasFlag(change, RES_CHG_TopOrder))
		{
			packet->writeInt(topOrder, Base::Bit16);
			packet->writeInt(topOrderTime, Base::Bit32);
		}
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* packet)
	{
		change = packet->readInt(Base::Bit8);
		if (HasFlag(change, RES_CHG_Money))
			money = packet->readInt(Base::Bit32);
		if (HasFlag(change, RES_CHG_Honor))
			honor = packet->readInt(Base::Bit32);
		if (HasFlag(change, RES_CHG_Activity))
			activity = packet->readInt(Base::Bit32);
		if (HasFlag(change, RES_CHG_MaxHonor))
		{
			maxHonor = packet->readInt(Base::Bit32);
			maxHonorTime = packet->readInt(Base::Bit32);
		}
		if (HasFlag(change, RES_CHG_MaxActivity))
		{
			maxActivity = packet->readInt(Base::Bit32);
			maxActivityTime = packet->readInt(Base::Bit32);
		}
		if (HasFlag(change, RES_CHG_TradeCount))
			tradeCount = packet->readInt(Base::Bit32);
		if (HasFlag(change, RES_CHG_Order))
			order = packet->readInt(Base::Bit16);
		if (HasFlag(change, RES_CHG_TopOrder))
		{
			topOrder = packet->readInt(Base::Bit16);
			topOrderTime = packet->readInt(Base::Bit32);
		}
	}
};

struct OrgAssaultStat
{
	enum ASSAULTSTAT_CHANGE
	{
		ASSAULTSTAT_CHG_NULL = 0,
		ASSAULTSTAT_CHG_KillCount = BIT(0),
		ASSAULTSTAT_CHG_BeKillCount = BIT(1)
	};
	int		curKillCount;
	int		curBeKillCount;
	int		maxKillCount;
	int		maxKillCountTime;
	char	maxKillCountOrg[COMMON_STRING_LENGTH];
	int		maxBeKillCount;
	int		maxBeKillCountTime;
	char	maxBeKillCountOrg[COMMON_STRING_LENGTH];
	int		totalKillCount;
	int		totalBeKillCount;
	int		assaultCount;
	int		winCount;
	int		loseCount;
	int		standoffCount;

	OrgAssaultStat()
	{
		memset(this, 0, sizeof(OrgAssaultStat));
		dStrcpy(maxKillCountOrg, sizeof(maxKillCountOrg), "");
		dStrcpy(maxBeKillCountOrg, sizeof(maxBeKillCountOrg), "");
	}

	OrgAssaultStat(const OrgAssaultStat& other)
	{
		*this = other;
	}

	OrgAssaultStat& operator=(const OrgAssaultStat& other)
	{
		memcpy(this, &other, sizeof(OrgAssaultStat));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(OrgAssaultStat) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(OrgAssaultStat) << 3, this);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* packet) const
	{
		packet->writeInt(change, Base::Bit8);
		if (HasFlag(change, ASSAULTSTAT_CHG_KillCount))
		{
			packet->writeInt(curKillCount, Base::Bit16);
			packet->writeInt(totalKillCount, Base::Bit32);
		}
		if (HasFlag(change, ASSAULTSTAT_CHG_BeKillCount))
		{
			packet->writeInt(curBeKillCount, Base::Bit16);
			packet->writeInt(totalBeKillCount, Base::Bit32);
		}
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* packet)
	{
		change = packet->readInt(Base::Bit8);
		if (HasFlag(change, ASSAULTSTAT_CHG_KillCount))
		{
			curKillCount = packet->readInt(Base::Bit16);
			totalKillCount = packet->readInt(Base::Bit32);
		}
		if (HasFlag(change, ASSAULTSTAT_CHG_BeKillCount))
		{
			curBeKillCount = packet->readInt(Base::Bit16);
			totalBeKillCount = packet->readInt(Base::Bit32);
		}
	}
};

struct Org
{
	enum
	{
		_maxMsgSize = 512,
		_maxMemoSize = 64,
		_maxLevel = 3,
		_maxDepot = 200,
		_maxBoss = 100,
		_maxMemberLevel = 5,
		_MaxBossLevel = 5,
		_maxPutOutMissionNum = 8
	};

	enum OrgPacket
	{
		ORGPACK_Min,
		ORGPACK_Mid,
		ORGPACK_Full
	};

	static int GetMemLimit(int level)
	{
		static int limits[_maxLevel] = { 36, 48, 72 };
		IF_ASSERT(!(level > 0 && level <= _countof(limits)))
			level = 1;
		return limits[level - 1];
	}

	static int GetMinActivity(int level)
	{
		static int activity[_maxLevel] = { 10000, 20000, 30000 };
		IF_ASSERT(!(level > 0 && level <= _countof(activity)))
			level = 1;
		return activity[level - 1];
	}

	static int GetDepotLimit(int level)
	{
		static int limits[_maxLevel] = { 30, 60, 90 };
		IF_ASSERT(!(level > 0 && level <= _countof(limits)))
			level = 1;
		return limits[level - 1];
	}

	static int GetLevelUpHonor(int level)
	{
		static int honor[_maxLevel] = { 10000, 100000, 0 };
		IF_ASSERT(!(level > 0 && level <= _countof(honor)))
			level = 1;
		return honor[level - 1];
	}

	static int GetDisbantTime(int level)
	{
		static int time[_maxLevel] = { 24 * 60 * 60, 2 * 24 * 60 * 60, 3 * 24 * 60 * 60 };
		IF_ASSERT(!(level > 0 && level <= _countof(time)))
			level = 1;
		return time[level - 1];
	}

	static int GetTradeRightLimit(int level)
	{
		static int limits[_maxLevel] = { 0, 2, 4 };
		IF_ASSERT(!(level > 0 && level <= _countof(limits)))
			level = 1;
		return limits[level - 1];
	}

	static int GetTaxItem(int income)
	{
		int value = ORG_TAX_ITEM_VALUE;
		int step = ORG_TAX_ITEM_VALUE;
		for (int i = 0; i < ORG_MAX_TAX_ITEM; ++i)
		{
			if (value >= 10000000)
				step = 1000000;
			else if (value >= 1000000)
				step = 500000;
			else if (value >= 100000)
				step = 100000;
			value += step;
			if (income < value)
				return ORG_TAX_ITEM + i;
		}
		JUST_ASSERT("GetTaxItem: income overflow.");
		return ORG_TAX_ITEM;
	}

	static int GetItemTax(int item)
	{
		int index = item - ORG_TAX_ITEM;
		IF_ASSERT(index >= ORG_MAX_TAX_ITEM)
			return ORG_TAX_ITEM_VALUE;
		int value = ORG_TAX_ITEM_VALUE;
		int step = ORG_TAX_ITEM_VALUE;
		for (int i = 0; i < index; ++i)
		{
			if (value >= 10000000)
				step = 1000000;
			else if (value >= 1000000)
				step = 500000;
			else if (value >= 100000)
				step = 100000;
			value += step;
		}
		return value;
	}

	static bool IsApplyDemesneTime(const tm& time)
	{
		return time.tm_wday >= 1 && time.tm_wday <= 5 && time.tm_wday != 3;
	}

	enum ORG_CHANGE
	{
		ORG_CHG_NULL = 0,
		ORG_CHG_Level = BIT(0),
		ORG_CHG_Password = BIT(1),
		ORG_CHG_Msg = BIT(2),
		ORG_CHG_MasterID = BIT(3),
		ORG_CHG_AutoAccept = BIT(4),
		ORG_CHG_BossID = BIT(5),
		ORG_CHG_DisbandTime = BIT(6),
		ORG_CHG_LeagueID = BIT(7),
		ORG_CHG_Memo = BIT(8),
		ORG_CHG_YYChannel = BIT(9),
		ORG_CHG_LastCallBossTime = BIT(10),
		ORG_CHG_SetSubMasterTime = BIT(11),
		ORG_CHG_PutOutMission = BIT(12),
	};

	int     id;
	int		level;
	char    name[COMMON_STRING_LENGTH];
	char	password[COMMON_STRING_LENGTH];
	char    msg[_maxMsgSize];
	int		masterId;
	bool	autoAcceptJoin;
	int		bossId;
	int		disbandTime;
	int		leagueId;
	int		changeLeagueTime;
	char	memo[_maxMemoSize];
	int		yyChannel;
	int		transferTime;
	int		lastCallBossTime;
	int		setSubMasterTime;
	int		putOutMissionTime[_maxPutOutMissionNum];//发布任务时间

	Org()
		:id(0), level(1), masterId(0), autoAcceptJoin(false),
		bossId(0), disbandTime(0), leagueId(0), changeLeagueTime(0), yyChannel(0), transferTime(0), lastCallBossTime(0), setSubMasterTime(0)
	{
		dStrcpy(name, sizeof(name), "");
		dStrcpy(password, sizeof(password), "");
		dStrcpy(msg, sizeof(msg), "");
		dStrcpy(memo, sizeof(memo), "");
		for (int i = 0; i < _maxPutOutMissionNum; ++i)
			putOutMissionTime[i] = 0;
	}

	Org(const Org& other)
	{
		*this = other;
	}

	Org& operator=(const Org& other)
	{
		memcpy(this, &other, sizeof(Org));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(id, Base::Bit32);
		stream->writeInt(level, Base::Bit8);
		stream->writeString(name, _countof(name));
		stream->writeString(password, _countof(password));
		stream->writeStringEx(msg, _countof(msg));
		stream->writeInt(masterId, Base::Bit32);
		stream->writeFlag(autoAcceptJoin);
		stream->writeInt(bossId, Base::Bit32);
		stream->writeInt(disbandTime, Base::Bit32);
		stream->writeInt(leagueId, Base::Bit32);
		stream->writeInt(changeLeagueTime, Base::Bit32);
		stream->writeString(memo, _countof(memo));
		stream->writeInt(yyChannel, Base::Bit32);
		stream->writeInt(transferTime, Base::Bit32);
		stream->writeInt(lastCallBossTime, Base::Bit32);
		stream->writeInt(setSubMasterTime, Base::Bit32);
		for (int i = 0; i < _maxPutOutMissionNum; ++i)
			stream->writeInt(putOutMissionTime[i], Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* stream)
	{
		id = stream->readInt(Base::Bit32);
		level = stream->readInt(Base::Bit8);
		stream->readString(name, _countof(name));
		stream->readString(password, _countof(password));
		stream->readStringEx(msg, _countof(msg));
		masterId = stream->readInt(Base::Bit32);
		autoAcceptJoin = stream->readFlag();
		bossId = stream->readInt(Base::Bit32);
		disbandTime = stream->readInt(Base::Bit32);
		leagueId = stream->readInt(Base::Bit32);
		changeLeagueTime = stream->readInt(Base::Bit32);
		stream->readString(memo, _countof(memo));
		yyChannel = stream->readInt(Base::Bit32);
		transferTime = stream->readInt(Base::Bit32);
		lastCallBossTime = stream->readInt(Base::Bit32);
		setSubMasterTime = stream->readInt(Base::Bit32);
		for (int i = 0; i < _maxPutOutMissionNum; ++i)
			putOutMissionTime[i] = stream->readInt(Base::Bit32);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* stream) const
	{
		stream->writeInt(change, Base::Bit16);
		if (HasFlag(change, ORG_CHG_Level))
			stream->writeInt(level, Base::Bit8);
		if (HasFlag(change, ORG_CHG_Password))
			stream->writeString(password, _countof(password));
		if (HasFlag(change, ORG_CHG_Msg))
			stream->writeStringEx(msg, _countof(msg));
		if (HasFlag(change, ORG_CHG_MasterID))
		{
			stream->writeInt(masterId, Base::Bit32);
			stream->writeInt(transferTime, Base::Bit32);
		}
		if (HasFlag(change, ORG_CHG_AutoAccept))
			stream->writeFlag(autoAcceptJoin);
		if (HasFlag(change, ORG_CHG_BossID))
			stream->writeInt(bossId, Base::Bit32);
		if (HasFlag(change, ORG_CHG_DisbandTime))
			stream->writeInt(disbandTime, Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueID))
		{
			stream->writeInt(leagueId, Base::Bit32);
			stream->writeInt(changeLeagueTime, Base::Bit32);
		}
		if (HasFlag(change, ORG_CHG_Memo))
			stream->writeString(memo, _countof(memo));
		if (HasFlag(change, ORG_CHG_YYChannel))
			stream->writeInt(yyChannel, Base::Bit32);
		if (HasFlag(change, ORG_CHG_LastCallBossTime))
			stream->writeInt(lastCallBossTime, Base::Bit32);
		if (HasFlag(change, ORG_CHG_SetSubMasterTime))
			stream->writeInt(setSubMasterTime, Base::Bit32);
		if (HasFlag(change, ORG_CHG_PutOutMission))
			for (int i = 0; i < _maxPutOutMissionNum; ++i)
				stream->writeInt(putOutMissionTime[i], Base::Bit32);
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* stream)
	{
		change = stream->readInt(Base::Bit16);
		if (HasFlag(change, ORG_CHG_Level))
			level = stream->readInt(Base::Bit8);
		if (HasFlag(change, ORG_CHG_Password))
			stream->readString(password, _countof(password));
		if (HasFlag(change, ORG_CHG_Msg))
			stream->readStringEx(msg, _countof(msg));
		if (HasFlag(change, ORG_CHG_MasterID))
		{
			masterId = stream->readInt(Base::Bit32);
			transferTime = stream->readInt(Base::Bit32);
		}
		if (HasFlag(change, ORG_CHG_AutoAccept))
			autoAcceptJoin = stream->readFlag();
		if (HasFlag(change, ORG_CHG_BossID))
			bossId = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_DisbandTime))
			disbandTime = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueID))
		{
			leagueId = stream->readInt(Base::Bit32);
			changeLeagueTime = stream->readInt(Base::Bit32);
		}
		if (HasFlag(change, ORG_CHG_Memo))
			stream->readString(memo, _countof(memo));
		if (HasFlag(change, ORG_CHG_YYChannel))
			yyChannel = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_LastCallBossTime))
			lastCallBossTime = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_SetSubMasterTime))
			setSubMasterTime = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_PutOutMission))
			for (int i = 0; i < _maxPutOutMissionNum; ++i)
				putOutMissionTime[i] = stream->readInt(Base::Bit32);
	}
};

struct ZoneOrg
{
	enum ORG_CHANGE
	{
		ORG_CHG_NULL = 0,
		ORG_CHG_Level = BIT(0),
		ORG_CHG_Honor = BIT(1),
		ORG_CHG_Master = BIT(2),
		ORG_CHG_Money = BIT(3),
		ORG_CHG_Activity = BIT(4),
		ORG_CHG_TradeCount = BIT(5),
		ORG_CHG_LeagueID = BIT(6),
		ORG_CHG_PutOutMission = BIT(7),
		ORG_CHG_AwardMoney = BIT(8),
	};

	int id;
	int level;
	char name[COMMON_STRING_LENGTH];
	int honor;
	int master;
	int money;
	int activity;
	int tradeCount;
	int leagueID;
	int	putOutMissionTime[Org::_maxPutOutMissionNum];//发布任务时间

	ZoneOrg()
	{
		memset(this, 0, sizeof(ZoneOrg));
		dStrcpy(name, sizeof(name), "");
	}

	ZoneOrg(const Org& org, const OrgRes& res)
		:id(org.id), level(org.level), honor(res.honor), master(org.masterId), money(res.money),
		activity(res.activity), tradeCount(res.tradeCount), leagueID(org.leagueId)
	{
		dStrcpy(name, sizeof(name), org.name);
		for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
			putOutMissionTime[i] = org.putOutMissionTime[i];
	}

	ZoneOrg(const ZoneOrg& zoneOrg)
	{
		*this = zoneOrg;
	}

	ZoneOrg& operator=(const ZoneOrg& zoneOrg)
	{
		memcpy(this, &zoneOrg, sizeof(ZoneOrg));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(id, Base::Bit32);
		stream->writeInt(level, Base::Bit8);
		stream->writeString(name, _countof(name));
		stream->writeInt(honor, Base::Bit32);
		stream->writeInt(master, Base::Bit32);
		stream->writeInt(money, Base::Bit32);
		stream->writeInt(activity, Base::Bit32);
		stream->writeInt(tradeCount, Base::Bit32);
		stream->writeInt(leagueID, Base::Bit32);
		for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
			stream->writeInt(putOutMissionTime[i], Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* stream)
	{
		id = stream->readInt(Base::Bit32);
		level = stream->readInt(Base::Bit8);
		stream->readString(name, _countof(name));
		honor = stream->readInt(Base::Bit32);
		master = stream->readInt(Base::Bit32);
		money = stream->readInt(Base::Bit32);
		activity = stream->readInt(Base::Bit32);
		tradeCount = stream->readInt(Base::Bit32);
		leagueID = stream->readInt(Base::Bit32);
		for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
			putOutMissionTime[i] = stream->readInt(Base::Bit32);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* stream) const
	{
		stream->writeInt(change, Base::Bit8);
		if (HasFlag(change, ORG_CHG_Level))
			stream->writeInt(level, Base::Bit8);
		if (HasFlag(change, ORG_CHG_Honor))
			stream->writeInt(honor, Base::Bit32);
		if (HasFlag(change, ORG_CHG_Master))
			stream->writeInt(master, Base::Bit32);
		if (HasFlag(change, ORG_CHG_Money) || HasFlag(change, ORG_CHG_AwardMoney))
			stream->writeInt(money, Base::Bit32);
		if (HasFlag(change, ORG_CHG_Activity))
			stream->writeInt(activity, Base::Bit32);
		if (HasFlag(change, ORG_CHG_TradeCount))
			stream->writeInt(tradeCount, Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueID))
			stream->writeInt(leagueID, Base::Bit32);
		if (HasFlag(change, ORG_CHG_PutOutMission))
			for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
				stream->writeInt(putOutMissionTime[i], Base::Bit32);
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* stream)
	{
		change = stream->readInt(Base::Bit8);
		if (HasFlag(change, ORG_CHG_Level))
			level = stream->readInt(Base::Bit8);
		if (HasFlag(change, ORG_CHG_Honor))
			honor = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_Master))
			master = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_Money) || HasFlag(change, ORG_CHG_AwardMoney))
			money = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_Activity))
			activity = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_TradeCount))
			tradeCount = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueID))
			leagueID = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_PutOutMission))
			for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
				putOutMissionTime[i] = stream->readInt(Base::Bit32);
	}
};

struct OrgApply
{
	int  playerId;
	char szPlayerName[COMMON_STRING_LENGTH];
	U8	 sex;
	U8	 level;
	U8	 nFamily;
	U8	 firstClass;

	OrgApply()
	{
		memset(this, 0, sizeof(OrgApply));
		dStrcpy(szPlayerName, sizeof(szPlayerName), "");
	}

	OrgApply(const OrgApply& other)
	{
		*this = other;
	}

	OrgApply& operator=(const OrgApply& other)
	{
		memcpy(this, &other, sizeof(OrgApply));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(playerId, Base::Bit32);
		packet->writeInt(level, Base::Bit8);
		packet->writeString(szPlayerName, _countof(szPlayerName));
		packet->writeInt(nFamily, Base::Bit8);
		packet->writeInt(sex, Base::Bit8);
		packet->writeInt(firstClass, Base::Bit8);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		playerId = packet->readInt(Base::Bit32);
		level = packet->readInt(Base::Bit8);
		packet->readString(szPlayerName, _countof(szPlayerName));
		nFamily = packet->readInt(Base::Bit8);
		sex = packet->readInt(Base::Bit8);
		firstClass = packet->readInt(Base::Bit8);
	}
};

struct OrgDepot
{
	int itemId;
	U8 amount;
	bool autoMake;
	int	makeCount;
	int consumeMoney;
	int buyCount;

	OrgDepot(int id, int money)
		:itemId(id), amount(0), autoMake(false), makeCount(0), consumeMoney(money), buyCount(0)
	{
	}

	OrgDepot()
	{
		memset(this, 0, sizeof(OrgDepot));
	}

	OrgDepot(const OrgDepot& other)
	{
		*this = other;
	}

	OrgDepot& operator=(const OrgDepot& other)
	{
		memcpy(this, &other, sizeof(OrgDepot));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(OrgDepot) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(OrgDepot) << 3, this);
	}
};

struct OrgBoss
{
	int bossID;
	int nimbus;
	bool cumulate;
	int callCount;
	int killCount;

	OrgBoss(int id, bool cum)
		:bossID(id), nimbus(0), cumulate(cum), callCount(0), killCount(0)
	{
	}

	OrgBoss()
	{
		memset(this, 0, sizeof(OrgBoss));
	}

	OrgBoss(const OrgBoss& other)
	{
		*this = other;
	}

	OrgBoss& operator=(const OrgBoss& other)
	{
		memcpy(this, &other, sizeof(OrgBoss));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(bossID, Base::Bit32);
		packet->writeInt(nimbus, Base::Bit32);
		packet->writeFlag(cumulate);
		packet->writeInt(callCount, Base::Bit16);
		packet->writeInt(killCount, Base::Bit16);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		bossID = packet->readInt(Base::Bit32);
		nimbus = packet->readInt(Base::Bit32);
		cumulate = packet->readFlag();
		callCount = packet->readInt(Base::Bit16);
		killCount = packet->readInt(Base::Bit16);
	}
};

struct OrgAssault
{
	int orgID;
	char orgName[COMMON_STRING_LENGTH];
	int assaultOrgID;
	char assaultOrgName[COMMON_STRING_LENGTH];
	int assaultTime;

	OrgAssault()
	{
		memset(this, 0, sizeof(OrgAssault));
		dStrcpy(orgName, sizeof(orgName), "");
		dStrcpy(assaultOrgName, sizeof(assaultOrgName), "");
	}

	OrgAssault(const OrgAssault& other)
	{
		*this = other;
	}

	OrgAssault& operator=(const OrgAssault& other)
	{
		memcpy(this, &other, sizeof(OrgAssault));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(orgID, Base::Bit32);
		packet->writeString(orgName, sizeof(orgName));
		packet->writeInt(assaultOrgID, Base::Bit32);
		packet->writeString(assaultOrgName, sizeof(assaultOrgName));
		packet->writeInt(assaultTime, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		orgID = packet->readInt(Base::Bit32);
		packet->readString(orgName, sizeof(orgName));
		assaultOrgID = packet->readInt(Base::Bit32);
		packet->readString(assaultOrgName, sizeof(assaultOrgName));
		assaultTime = packet->readInt(Base::Bit32);
	}
};

enum DEMESNE_STATE
{
	DEMESTATE_Apply,
	DEMESTATE_Decide,
	DEMESTATE_Occupy
};

struct OrgDemesne
{
	enFamily family;
	int orgID;
	char orgName[COMMON_STRING_LENGTH];
	DEMESNE_STATE flag;
	int count;
	char masterName[COMMON_STRING_LENGTH];

	int applyCount;
	int totalApplyCount;

	OrgDemesne(enFamily family, int orgID, DEMESNE_STATE flag, const char* name, const char* master)
		:family(family), orgID(orgID), flag(flag), count(0), applyCount(0), totalApplyCount(0)
	{
		dStrcpy(orgName, sizeof(orgName), name);
		dStrcpy(masterName, sizeof(masterName), master);
	}

	OrgDemesne(enFamily family, DEMESNE_STATE flag, int count)
		:family(family), flag(flag), count(count), orgID(0), applyCount(0), totalApplyCount(0)
	{
		dStrcpy(orgName, sizeof(orgName), "");
		dStrcpy(masterName, sizeof(masterName), "");
	}

	OrgDemesne()
	{
		memset(this, 0, sizeof(OrgDemesne));
		dStrcpy(orgName, sizeof(orgName), "");
		dStrcpy(masterName, sizeof(masterName), "");
	}

	OrgDemesne(const OrgDemesne& other)
	{
		*this = other;
	}

	OrgDemesne& operator=(const OrgDemesne& other)
	{
		memcpy(this, &other, sizeof(OrgDemesne));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(family, Base::Bit8);
		packet->writeInt(flag, Base::Bit8);
		switch (flag)
		{
		case DEMESTATE_Apply:
			packet->writeInt(count, Base::Bit16);
			packet->writeInt(applyCount, Base::Bit16);
			packet->writeInt(totalApplyCount, Base::Bit16);
			break;
		case DEMESTATE_Decide:
			packet->writeInt(orgID, Base::Bit32);
			packet->writeString(orgName, sizeof(orgName));
			break;
		case DEMESTATE_Occupy:
			packet->writeInt(orgID, Base::Bit32);
			packet->writeString(orgName, sizeof(orgName));
			packet->writeString(masterName, sizeof(masterName));
			break;
		default:
			JUST_ASSERT("illegal OrgDemesne flag.");
			break;
		}
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		family = (enFamily)packet->readInt(Base::Bit8);
		flag = (DEMESNE_STATE)packet->readInt(Base::Bit8);
		switch (flag)
		{
		case DEMESTATE_Apply:
			count = packet->readInt(Base::Bit16);
			applyCount = packet->readInt(Base::Bit16);
			totalApplyCount = packet->readInt(Base::Bit16);
			orgID = 0;
			dStrcpy(orgName, sizeof(orgName), "");
			dStrcpy(masterName, sizeof(masterName), "");
			break;
		case DEMESTATE_Decide:
			orgID = packet->readInt(Base::Bit32);
			packet->readString(orgName, sizeof(orgName));
			count = 0;
			dStrcpy(masterName, sizeof(masterName), "");
			break;
		case DEMESTATE_Occupy:
			orgID = packet->readInt(Base::Bit32);
			packet->readString(orgName, sizeof(orgName));
			packet->readString(masterName, sizeof(masterName));
			count = 0;
			break;
		default:
			JUST_ASSERT("illegal OrgDemesne flag.");
			break;
		}
	}
};

enum ORG_INCOME
{
	ORG_INC_Today_Worship,
	ORG_INC_Yesterday_Worship,
	ORG_INC_Week_Shop,
	ORG_INC_LastWeek_Shop,
	ORG_INC_Week_Booth,
	ORG_INC_LastWeek_Booth,
	ORG_INC_Week_Stall,
	ORG_INC_LastWeek_Stall,
	ORG_INC_Week_Skill,
	ORG_INC_LastWeek_Skill
};

struct OrgDemesneIncome
{
	enum ORGDEMESNE_PACK
	{
		ORGDEMESNE_None = 0,
		ORGDEMESNE_Income = BIT(0),
		ORGDEMESNE_Effigy = BIT(1)
	};

	enFamily family;
	int todayWorship;
	int yesterdayWorship;
	int weekShopTax;
	int weekBoothTax;
	int weekStallTax;
	int weekSkillTax;
	int lastWeekShopTax;
	int lastWeekBoothTax;
	int lastWeekStallTax;
	int lastWeekSkillTax;
	int lastWeekWage;
	int effigyID;
	char effigyName[ORG_MAX_EFFGYNAME];
	int applyCount;

	OrgDemesneIncome()
	{
		memset(this, 0, sizeof(OrgDemesneIncome));
		dStrcpy(effigyName, sizeof(effigyName), "");
	}

	OrgDemesneIncome(enFamily familyID, int id, const char* name)
	{
		memset(this, 0, sizeof(OrgDemesneIncome));
		family = familyID;
		effigyID = id;
		dStrcpy(effigyName, sizeof(effigyName), name);
	}

	OrgDemesneIncome(const OrgDemesneIncome& other)
	{
		*this = other;
	}

	OrgDemesneIncome& operator=(const OrgDemesneIncome& other)
	{
		memcpy(this, &other, sizeof(OrgDemesneIncome));
		return *this;
	}

	void Clear()
	{
		todayWorship = 0;
		yesterdayWorship = 0;
		weekShopTax = 0;
		weekBoothTax = 0;
		weekStallTax = 0;
		weekSkillTax = 0;
		lastWeekShopTax = 0;
		lastWeekBoothTax = 0;
		lastWeekStallTax = 0;
		lastWeekSkillTax = 0;
		lastWeekWage = 0;
	}

	OrgDemesneIncome& operator+=(const OrgDemesneIncome& other)
	{
		todayWorship += other.todayWorship;
		yesterdayWorship += other.yesterdayWorship;
		weekShopTax += other.weekShopTax;
		weekBoothTax += other.weekBoothTax;
		weekStallTax += other.weekStallTax;
		weekSkillTax += other.weekSkillTax;
		lastWeekShopTax += other.lastWeekShopTax;
		lastWeekBoothTax += other.lastWeekBoothTax;
		lastWeekStallTax += other.lastWeekStallTax;
		lastWeekSkillTax += other.lastWeekSkillTax;
		lastWeekWage += other.lastWeekWage;
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet, int mode) const
	{
		packet->writeInt(mode, Base::Bit8);
		packet->writeInt(family, Base::Bit8);
		if (HasFlag(mode, ORGDEMESNE_Income))
		{
			packet->writeInt(todayWorship, Base::Bit32);
			packet->writeInt(yesterdayWorship, Base::Bit32);
			packet->writeInt(weekShopTax, Base::Bit32);
			packet->writeInt(weekBoothTax, Base::Bit32);
			packet->writeInt(weekStallTax, Base::Bit32);
			packet->writeInt(weekSkillTax, Base::Bit32);
			packet->writeInt(lastWeekShopTax, Base::Bit32);
			packet->writeInt(lastWeekBoothTax, Base::Bit32);
			packet->writeInt(lastWeekStallTax, Base::Bit32);
			packet->writeInt(lastWeekSkillTax, Base::Bit32);
			packet->writeInt(lastWeekWage, Base::Bit32);
		}
		if (HasFlag(mode, ORGDEMESNE_Effigy))
		{
			packet->writeInt(effigyID, Base::Bit32);
			packet->writeString(effigyName, sizeof(effigyName));
		}
	}

	template<typename _Ty>
	void ReadData(_Ty* packet, int& mode)
	{
		mode = packet->readInt(Base::Bit8);
		family = (enFamily)packet->readInt(Base::Bit8);
		if (HasFlag(mode, ORGDEMESNE_Income))
		{
			todayWorship = packet->readInt(Base::Bit32);
			yesterdayWorship = packet->readInt(Base::Bit32);
			weekShopTax = packet->readInt(Base::Bit32);
			weekBoothTax = packet->readInt(Base::Bit32);
			weekStallTax = packet->readInt(Base::Bit32);
			weekSkillTax = packet->readInt(Base::Bit32);
			lastWeekShopTax = packet->readInt(Base::Bit32);
			lastWeekBoothTax = packet->readInt(Base::Bit32);
			lastWeekStallTax = packet->readInt(Base::Bit32);
			lastWeekSkillTax = packet->readInt(Base::Bit32);
			lastWeekWage = packet->readInt(Base::Bit32);
		}
		if (HasFlag(mode, ORGDEMESNE_Effigy))
		{
			effigyID = packet->readInt(Base::Bit32);
			packet->readString(effigyName, sizeof(effigyName));
		}
	}

	void ReadUpdate(const OrgDemesneIncome& other, int mode)
	{
		family = other.family;
		if (HasFlag(mode, ORGDEMESNE_Income))
		{
			todayWorship = other.todayWorship;
			yesterdayWorship = other.yesterdayWorship;
			weekShopTax = other.weekShopTax;
			weekBoothTax = other.weekShopTax;
			weekStallTax = other.weekShopTax;
			weekSkillTax = other.weekSkillTax;
			lastWeekShopTax = other.lastWeekShopTax;
			lastWeekBoothTax = other.lastWeekBoothTax;
			lastWeekStallTax = other.lastWeekStallTax;
			lastWeekSkillTax = other.lastWeekSkillTax;
			lastWeekWage = other.lastWeekWage;
		}
		if (HasFlag(mode, ORGDEMESNE_Effigy))
		{
			effigyID = other.effigyID;
			dStrcpy(effigyName, sizeof(effigyName), other.effigyName);
		}
	}
};

enum OrgError
{
	ORGOP_OK,
	ORGERR_OrgExist,
	ORGERR_DBHandleFail,
	ORGERR_NotMember,
	ORGERR_HasnotOrg,
	ORGERR_IsMember,
	ORGERR_HasOrg,
	ORGERR_NoEnoughMoney,
	ORGERR_Offline,
	ORGERR_Reject,
	ORGERR_MaxMember,
	ORGERR_CannotRemoveMaster,
	ORGERR_NotMaster,
	ORGERR_MasterCannotLeave,
	ORGERR_DepotExist,
	ORGERR_DepotNotExist,
	ORGERR_OrgNotExist,
	ORGERR_AlreadyApply,
	ORGERR_AlreadyStudy,
	ORGERR_HasnotStudy,
	ORGERR_NotStudyPreItem,
	ORGERR_NoEnoughOrgMoney,
	ORGERR_LowLevel,
	ORGERR_ZeroAmount,
	ORGERR_ManufactureAmountLimit,
	ORGERR_NotManufacturePreItem,
	ORGERR_AmountLimit,
	ORGERR_NotFitClass,
	ORGERR_MemberNoEnoughOrgMoney,
	ORGERR_MemberLowLevel,
	ORGERR_NoEnoughConfirm,
	ORGERR_NoPassword,
	ORGERR_NoTeam,
	ORGERR_NotTeamCaption,
	ORGERR_TeamCaptionLimit,
	ORGERR_TeamLimit,
	ORGERR_OutofApply,
	ORGERR_NoName,
	ORGERR_BadName,
	ORGERR_MaxLevel,
	ORGERR_CaptionCancelCreate,
	ORGERR_PlayerLowLevel,
	ORGERR_NotTimeGetPay,
	ORGERR_BossExist,
	ORGERR_BossNotExist,
	ORGERR_BossCannotCumulate,
	ORGERR_BossAlreadyCumulate,
	ORGERR_NoCumulateItem,
	ORGERR_BossCumulateFull,
	ORGERR_BossCumulateNotFull,
	ORGERR_BossAlreadyCanCumulate,
	ORGERR_NoConsumeItem,
	ORGERR_CannotTrade,
	ORGERR_OnlyOneItem,
	ORGERR_AlreadyDisband,
	ORGERR_NotDisband,
	ORGERR_InventoryNoEnoughSpace,
	ORGERR_MemberAreadyDel,
	ORGERR_TransferTimeLimit,
	ORGERR_MasterCannotVote,
	ORGERR_AssaultDisbandOrg,
	ORGERR_AssaultSameLeagueOrg,
	ORGERR_AlreadyAssaultState,
	ORGERR_DisbandAssaultOrg,
	ORGERR_NotAssaultState,
	ORGERR_AssaultOrgError,
	ORGERR_MaxSubMaster,
	ORGERR_MasterToBeSubMaster,
	ORGERR_NotAuthorize,
	ORGERR_MakeInfinity,
	ORGERR_AlreadyApplyDemesne,
	ORGERR_NotApplyDemesne,
	ORGERR_ApplyDemesneDecide,
	ORGERR_NotApplyDemesneTime,
	ORGERR_AreadyDemesne,
	ORGERR_DisbandOrgApplyDemesne,
	ORGERR_ApplySameLeagueDemesne,
	ORGERR_CallBossTimeLimit,
	ORGERR_SetSubMasterTimeLimit,
	ORGERR_MasterHasTradeRight,
	ORGERR_TradeRightNumLimit,
	ORGERR_MemberNoEnoughLeagueMoney,
	ORGERR_JoinTimeLimit,
	ORGERR_CreatorJoinTimeLimit,
	ORGERR_OrgAwardMoneyIsZero,
	ORGERR_PutOutMissionTimeLimit,
	ORGERR_PutOutMissionNumLimit,
	ORGERR_MaxApllyNum,

	LEAGUEERR_LeagueExist,
	LEAGUEERR_HasLeague,
	LEAGUEERR_MaxOrg,
	LEAGUEERR_IsMember,
	LEAGUEERR_LeagueNotExist,
	LEAGUEERR_HasnotLeague,
	LEAGUEERR_MasterOffline,
	LEAGUEERR_CannotRemoveMaster,
	LEAGUEERR_NotMember,
	LEAGUEERR_MasterCannotLeave,
	LEAGUEERR_TeamLimit,
	LEAGUEERR_TeamCaptionLimit,
	LEAGUEERR_CaptionCancelCreate,
	LEAGUEERR_NoEnoughMoney,
	LEAGUEERR_NotTeamCaption,
	LEAGUEERR_NoEnoughConfirm,
	LEAGUEERR_NoName,
	LEAGUEERR_BadName,
	LEAGUEERR_Reject,
	LEAGUEERR_NoTeam,
	LEAGUEERR_NotMaster,
	LEAGUEERR_AreadyApply,
	LEAGUEERR_NotAuthorize,
	LEAGUEERR_OutofApply,
	LEAGUEERR_TransferTimeLimit,
	LEAGUEERR_MasterOrgCannotDisband,
	LEAGUEERR_TransferOrgDisband,
	LEAGUEERR_NotApplyDemesneTime,
	LEAGUEERR_ApplyDemesneDecide,
	LEAGUEERR_AreadyDemesne,
	LEAGUEERR_AlreadyApplyDemesne,
	LEAGUEERR_LowLevel,
	LEAGUEERR_OrgDemesneLimit,
	LEAGUEERR_AssaultOrgCreateLimit,
	LEAGUEERR_JoinTimeLimit,
	LEAGUEERR_CreatorJoinTimeLimit,
	LEAGUEERR_MaxApplyNum,

	ORGERR_Unknow,
	ORGERR_Count
};

static const char* strOrgError[] =
{
	ORGSTR(ORGOP_OK),
	ORGSTR(ORGERR_OrgExist),
	ORGSTR(ORGERR_DBHandleFail),
	ORGSTR(ORGERR_NotMember),
	ORGSTR(ORGERR_HasnotOrg),
	ORGSTR(ORGERR_IsMember),
	ORGSTR(ORGERR_HasOrg),
	ORGSTR(ORGERR_NoEnoughMoney),
	ORGSTR(ORGERR_Offline),
	ORGSTR(ORGERR_Reject),
	ORGSTR(ORGERR_MaxMember),
	ORGSTR(ORGERR_CannotRemoveMaster),
	ORGSTR(ORGERR_NotMaster),
	ORGSTR(ORGERR_MasterCannotLeave),
	ORGSTR(ORGERR_DepotExist),
	ORGSTR(ORGERR_DepotNotExist),
	ORGSTR(ORGERR_OrgNotExist),
	ORGSTR(ORGERR_AlreadyApply),
	ORGSTR(ORGERR_AlreadyStudy),
	ORGSTR(ORGERR_HasnotStudy),
	ORGSTR(ORGERR_NotStudyPreItem),
	ORGSTR(ORGERR_NoEnoughOrgMoney),
	ORGSTR(ORGERR_LowLevel),
	ORGSTR(ORGERR_ZeroAmount),
	ORGSTR(ORGERR_ManufactureAmountLimit),
	ORGSTR(ORGERR_NotManufacturePreItem),
	ORGSTR(ORGERR_AmountLimit),
	ORGSTR(ORGERR_NotFitClass),
	ORGSTR(ORGERR_MemberNoEnoughOrgMoney),
	ORGSTR(ORGERR_MemberLowLevel),
	ORGSTR(ORGERR_NoEnoughConfirm),
	ORGSTR(ORGERR_NoPassword),
	ORGSTR(ORGERR_NoTeam),
	ORGSTR(ORGERR_NotTeamCaption),
	ORGSTR(ORGERR_TeamCaptionLimit),
	ORGSTR(ORGERR_TeamLimit),
	ORGSTR(ORGERR_OutofApply),
	ORGSTR(ORGERR_NoName),
	ORGSTR(ORGERR_BadName),
	ORGSTR(ORGERR_MaxLevel),
	ORGSTR(ORGERR_CaptionCancelCreate),
	ORGSTR(ORGERR_PlayerLowLevel),
	ORGSTR(ORGERR_NotTimeGetPay),
	ORGSTR(ORGERR_BossExist),
	ORGSTR(ORGERR_BossNotExist),
	ORGSTR(ORGERR_BossCannotCumulate),
	ORGSTR(ORGERR_BossAlreadyCumulate),
	ORGSTR(ORGERR_NoCumulateItem),
	ORGSTR(ORGERR_BossCumulateFull),
	ORGSTR(ORGERR_BossCumulateNotFull),
	ORGSTR(ORGERR_BossAlreadyCanCumulate),
	ORGSTR(ORGERR_NoConsumeItem),
	ORGSTR(ORGERR_CannotTrade),
	ORGSTR(ORGERR_OnlyOneItem),
	ORGSTR(ORGERR_AlreadyDisband),
	ORGSTR(ORGERR_NotDisband),
	ORGSTR(ORGERR_InventoryNoEnoughSpace),
	ORGSTR(ORGERR_MemberAreadyDel),
	ORGSTR(ORGERR_TransferTimeLimit),
	ORGSTR(ORGERR_MasterCannotVote),
	ORGSTR(ORGERR_AssaultDisbandOrg),
	ORGSTR(ORGERR_AssaultSameLeagueOrg),
	ORGSTR(ORGERR_AlreadyAssaultState),
	ORGSTR(ORGERR_DisbandAssaultOrg),
	ORGSTR(ORGERR_NotAssaultState),
	ORGSTR(ORGERR_AssaultOrgError),
	ORGSTR(ORGERR_MaxSubMaster),
	ORGSTR(ORGERR_MasterToBeSubMaster),
	ORGSTR(ORGERR_NotAuthorize),
	ORGSTR(ORGERR_MakeInfinity),
	ORGSTR(ORGERR_AlreadyApplyDemesne),
	ORGSTR(ORGERR_NotApplyDemesne),
	ORGSTR(ORGERR_ApplyDemesneDecide),
	ORGSTR(ORGERR_NotApplyDemesneTime),
	ORGSTR(ORGERR_AreadyDemesne),
	ORGSTR(ORGERR_DisbandOrgApplyDemesne),
	ORGSTR(ORGERR_ApplySameLeagueDemesne),
	ORGSTR(ORGERR_CallBossTimeLimit),
	ORGSTR(ORGERR_SetSubMasterTimeLimit),
	ORGSTR(ORGERR_MasterHasTradeRight),
	ORGSTR(ORGERR_TradeRightNumLimit),
	ORGSTR(ORGERR_MemberNoEnoughLeagueMoney),
	ORGSTR(ORGERR_JoinTimeLimit),
	ORGSTR(ORGERR_CreatorJoinTimeLimit),
	ORGSTR(ORGERR_PutOutMissionTimeLimit),
	ORGSTR(ORGERR_PutOutMissionNumLimit),
	ORGSTR(ORGERR_MaxApllyNum),

	ORGSTR(LEAGUEERR_LeagueExist),
	ORGSTR(LEAGUEERR_HasLeague),
	ORGSTR(LEAGUEERR_MaxOrg),
	ORGSTR(LEAGUEERR_IsMember),
	ORGSTR(LEAGUEERR_LeagueNotExist),
	ORGSTR(LEAGUEERR_HasnotLeague),
	ORGSTR(LEAGUEERR_MasterOffline),
	ORGSTR(LEAGUEERR_CannotRemoveMaster),
	ORGSTR(LEAGUEERR_NotMember),
	ORGSTR(LEAGUEERR_MasterCannotLeave),
	ORGSTR(LEAGUEERR_TeamLimit),
	ORGSTR(LEAGUEERR_TeamCaptionLimit),
	ORGSTR(LEAGUEERR_CaptionCancelCreate),
	ORGSTR(LEAGUEERR_NoEnoughMoney),
	ORGSTR(LEAGUEERR_NotTeamCaption),
	ORGSTR(LEAGUEERR_NoEnoughConfirm),
	ORGSTR(LEAGUEERR_NoName),
	ORGSTR(LEAGUEERR_BadName),
	ORGSTR(LEAGUEERR_Reject),
	ORGSTR(LEAGUEERR_NoTeam),
	ORGSTR(LEAGUEERR_NotMaster),
	ORGSTR(LEAGUEERR_AreadyApply),
	ORGSTR(LEAGUEERR_NotAuthorize),
	ORGSTR(LEAGUEERR_OutofApply),
	ORGSTR(LEAGUEERR_TransferTimeLimit),
	ORGSTR(LEAGUEERR_MasterOrgCannotDisband),
	ORGSTR(LEAGUEERR_TransferOrgDisband),
	ORGSTR(LEAGUEERR_NotApplyDemesneTime),
	ORGSTR(LEAGUEERR_ApplyDemesneDecide),
	ORGSTR(LEAGUEERR_AreadyDemesne),
	ORGSTR(LEAGUEERR_AlreadyApplyDemesne),
	ORGSTR(LEAGUEERR_LowLevel),
	ORGSTR(LEAGUEERR_OrgDemesneLimit),
	ORGSTR(LEAGUEERR_AssaultOrgCreateLimit),
	ORGSTR(LEAGUEERR_JoinTimeLimit),
	ORGSTR(LEAGUEERR_CreatorJoinTimeLimit),
	ORGSTR(LEAGUEERR_MaxApplyNum),

	ORGSTR(ORGERR_Unknow)
};

#include <vector>

struct OrgDB
{
	struct OrgData
	{
		Org org;
		OrgMaster master;
		OrgRes res;
		OrgAssaultStat assaultStat;
	};
	struct MemberData
	{
		int orgID;
		bool hasDelete;
		int deleteTime;
		OrgMember orgMember;
	};
	struct ApplyData
	{
		int orgID;
		int playerID;
	};
	struct DepotData
	{
		int orgID;
		OrgDepot orgDepot;
	};
	struct BossData
	{
		int orgID;
		OrgBoss orgBoss;
	};
	typedef std::vector<OrgData> Orgs;
	typedef Orgs::iterator OrgsIter;
	typedef std::vector<MemberData> Members;
	typedef Members::iterator MembersIter;
	typedef std::vector<ApplyData> Applys;
	typedef Applys::iterator ApplysIter;
	typedef std::vector<DepotData> Depots;
	typedef Depots::iterator DepotsIter;
	typedef std::vector<BossData> Bosses;
	typedef Bosses::iterator BossesIter;
	typedef std::vector<OrgAssault> Assaults;
	typedef Assaults::iterator AssaultsIter;
	typedef std::vector<OrgDemesne> Demesnes;
	typedef Demesnes::iterator DemesnesIter;
	typedef std::vector<OrgDemesneIncome> Incomes;
	typedef Incomes::iterator IncomesIter;

	Orgs orgs;
	Members members;
	Applys applys;
	Depots depots;
	Bosses bosses;
	Assaults assaults;
	Demesnes demesnes;
	Incomes incomes;
};

#include <set>

typedef std::set<int> OrgIDSet;
typedef OrgIDSet::iterator OrgIDSetIter;
typedef OrgIDSet::const_iterator OrgIDSetConIter;

struct OrgCreate
{
	Org org;
	OrgMaster master;
	OrgIDSet creator;
};

typedef std::vector< std::pair<int, OrgError> > OrgDelApplys;
typedef OrgDelApplys::const_iterator OrgDelApplysIter;

struct OrgTopRank
{
	int orgID;
	int honor;
	int acitvity;
	int num;
	int level;
	char name[COMMON_STRING_LENGTH];
	int money;
	int bossVictory;
	int assaultVictory;

	OrgTopRank()
	{
		memset(this, 0, sizeof(OrgTopRank));
		dStrcpy(name, sizeof(name), "");
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(OrgTopRank) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(OrgTopRank) << 3, this);
	}
};

struct LeagueTopRank
{
	int leagueID;
	int honor;
	int num;
	int level;
	char name[COMMON_STRING_LENGTH];

	LeagueTopRank()
	{
		memset(this, 0, sizeof(LeagueTopRank));
		dStrcpy(name, sizeof(name), "");
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(LeagueTopRank) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(LeagueTopRank) << 3, this);
	}
};

typedef std::vector<OrgBoss> OrgBosses;
typedef OrgBosses::const_iterator OrgBossesIter;

struct League
{
	enum
	{
		_maxMsgSize = 1024,
		_maxOrg = 6
	};

	static int GetLevel(int totalLevel)
	{
		if (totalLevel >= 18)
			return 3;
		else if (totalLevel >= 9)
			return 2;
		else
			return 1;
	}

	static bool IsDemesne(int zoneID)
	{
		return false;
		static int zones[] = { 1001 };
		for (int i = 0; i < _countof(zones); ++i)
		{
			if (zones[i] == zoneID)
				return true;
		}
		return false;
	}

	static bool IsApplyDemesneTime(const tm& time)
	{
		return time.tm_wday >= 1 && time.tm_wday <= 4;
	}

	enum LEAGUE_CHG
	{
		LEAGUE_CHG_NULL = 0,
		LEAGUE_CHG_Msg = BIT(0),
		LEAGUE_CHG_MasterOrg = BIT(1),
		LEAGUE_CHG_AutoAccept = BIT(2),
		LEAGUE_CHG_SubMasterOrg = BIT(3),
		LEAGUE_CHG_YYChannel = BIT(4)
	};

	int     id;
	char    name[COMMON_STRING_LENGTH];
	char    msg[_maxMsgSize];
	int		masterOrg;
	bool	autoAcceptJoin;
	int		order;
	int		subMasterOrg;
	int		transferTime;
	int		yyChannel;

	League()
		:id(0), masterOrg(0), autoAcceptJoin(false), order(0),
		subMasterOrg(0), transferTime(0), yyChannel(0)
	{
		dStrcpy(name, sizeof(name), "");
		dStrcpy(msg, sizeof(msg), "");
	}

	League(const League& other)
	{
		*this = other;
	}

	League& operator=(const League& other)
	{
		memcpy(this, &other, sizeof(League));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(id, Base::Bit32);
		stream->writeString(name, _countof(name));
		stream->writeStringEx(msg, _countof(msg));
		stream->writeInt(masterOrg, Base::Bit32);
		stream->writeFlag(autoAcceptJoin);
		stream->writeInt(order, Base::Bit16);
		stream->writeInt(subMasterOrg, Base::Bit32);
		stream->writeInt(transferTime, Base::Bit32);
		stream->writeInt(yyChannel, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* stream)
	{
		id = stream->readInt(Base::Bit32);
		stream->readString(name, _countof(name));
		stream->readStringEx(msg, _countof(msg));
		masterOrg = stream->readInt(Base::Bit32);
		autoAcceptJoin = stream->readFlag();
		order = stream->readInt(Base::Bit16);
		subMasterOrg = stream->readInt(Base::Bit32);
		transferTime = stream->readInt(Base::Bit32);
		yyChannel = stream->readInt(Base::Bit32);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* stream) const
	{
		stream->writeInt(change, Base::Bit8);
		if (HasFlag(change, LEAGUE_CHG_Msg))
			stream->writeStringEx(msg, _countof(msg));
		if (HasFlag(change, LEAGUE_CHG_MasterOrg))
		{
			stream->writeInt(masterOrg, Base::Bit32);
			stream->writeInt(transferTime, Base::Bit32);
		}
		if (HasFlag(change, LEAGUE_CHG_AutoAccept))
			stream->writeFlag(autoAcceptJoin);
		if (HasFlag(change, LEAGUE_CHG_SubMasterOrg))
			stream->writeInt(subMasterOrg, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_YYChannel))
			stream->writeInt(yyChannel, Base::Bit32);
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* stream)
	{
		change = stream->readInt(Base::Bit8);
		if (HasFlag(change, LEAGUE_CHG_Msg))
			stream->readStringEx(msg, _countof(msg));
		if (HasFlag(change, LEAGUE_CHG_MasterOrg))
		{
			masterOrg = stream->readInt(Base::Bit32);
			transferTime = stream->readInt(Base::Bit32);
		}
		if (HasFlag(change, LEAGUE_CHG_AutoAccept))
			autoAcceptJoin = stream->readFlag();
		if (HasFlag(change, LEAGUE_CHG_SubMasterOrg))
			subMasterOrg = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_YYChannel))
			yyChannel = stream->readInt(Base::Bit32);
	}
};

struct LeagueRes
{
	enum RES_CHANGE
	{
		RES_CHG_NULL = 0,
		RES_CHG_Stone = BIT(0),
		RES_CHG_Sprite = BIT(1),
		RES_CHG_Wood = BIT(2),
		RES_CHG_Money = BIT(3)
	};

	int stone;
	int sprite;
	int food;
	int drug;
	int wood;
	int cloth;
	int paper;
	int money;

	LeagueRes()
	{
		memset(this, 0, sizeof(LeagueRes));
	}

	LeagueRes(const LeagueRes& other)
	{
		*this = other;
	}

	LeagueRes& operator=(const LeagueRes& other)
	{
		memcpy(this, &other, sizeof(LeagueRes));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(LeagueRes) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(LeagueRes) << 3, this);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* packet) const
	{
		packet->writeInt(change, Base::Bit8);
		if (HasFlag(change, RES_CHG_Stone))
			packet->writeInt(stone, Base::Bit32);
		if (HasFlag(change, RES_CHG_Sprite))
			packet->writeInt(sprite, Base::Bit32);
		if (HasFlag(change, RES_CHG_Wood))
			packet->writeInt(wood, Base::Bit32);
		if (HasFlag(change, RES_CHG_Money))
			packet->writeInt(money, Base::Bit32);
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* packet)
	{
		change = packet->readInt(Base::Bit8);
		if (HasFlag(change, RES_CHG_Stone))
			stone = packet->readInt(Base::Bit32);
		if (HasFlag(change, RES_CHG_Sprite))
			sprite = packet->readInt(Base::Bit32);
		if (HasFlag(change, RES_CHG_Wood))
			wood = packet->readInt(Base::Bit32);
		if (HasFlag(change, RES_CHG_Money))
			money = packet->readInt(Base::Bit32);
	}
};

struct OrgLogInfo
{
	struct OrgInfo
	{
		int orgID;
		int masterID;
		int money;
	} orgInfo;
	struct MemberInfo
	{
		int playerID;
		int money;
		int	awardMoney;
		int	awardCanTradeMoney;
		int leagueMoney;
	} memberInfo;
	struct OpInfo
	{
		int itemID;
		int itemAmount;
		int money;
		int result;
	} opInfo;

	OrgLogInfo()
	{
		Clear();
	}

	OrgLogInfo(const OrgLogInfo& other)
	{
		*this = other;
	}

	OrgLogInfo& operator=(const OrgLogInfo& other)
	{
		memcpy(this, &other, sizeof(OrgLogInfo));
		return *this;
	}

	void Clear()
	{
		memset(this, 0, sizeof(OrgLogInfo));
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(OrgLogInfo) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(OrgLogInfo) << 3, this);
	}
};

struct LeagueLogInfo
{
	struct LeagueInfo
	{
		int leagueID;
		int masterOrg;
		int subMasterOrg;
	} leagueInfo;
	struct OrgInfo
	{
		int orgID;
	} orgInfo;

	LeagueLogInfo()
	{
		Clear();
	}

	LeagueLogInfo(const LeagueLogInfo& other)
	{
		*this = other;
	}

	LeagueLogInfo& operator=(const LeagueLogInfo& other)
	{
		memcpy(this, &other, sizeof(LeagueLogInfo));
		return *this;
	}

	void Clear()
	{
		memset(this, 0, sizeof(LeagueLogInfo));
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(LeagueLogInfo) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(LeagueLogInfo) << 3, this);
	}
};

struct ZoneLeague
{
	enum LEAGUE_CHG
	{
		LEAGUE_CHG_NULL = 0,
		LEAGUE_CHG_Honor = BIT(0),
		LEAGUE_CHG_MasterOrg = BIT(1),
		LEAGUE_CHG_Level = BIT(2),
		LEAGUE_CHG_SubMasterOrg = BIT(3),
		LEAGUE_CHG_Stone = BIT(4),
		LEAGUE_CHG_Sprite = BIT(5),
		LEAGUE_CHG_Wood = BIT(6),
		LEAGUE_CHG_Money = BIT(7),
		LEAGUE_CHG_MasterID = BIT(8),
		LEAGUE_CHG_SubMasterID = BIT(9)
	};

	int id;
	char name[COMMON_STRING_LENGTH];
	int honor;
	int masterOrg;
	int level;
	int subMasterOrg;
	int stone;
	int sprite;
	int wood;
	int money;
	int masterID;
	int subMasterID;

	ZoneLeague()
	{
		memset(this, 0, sizeof(ZoneLeague));
		dStrcpy(name, sizeof(name), "");
	}

	ZoneLeague(const League& league, const LeagueRes& res)
		:id(league.id), masterOrg(league.masterOrg), subMasterOrg(league.subMasterOrg),
		stone(res.stone), sprite(res.sprite), wood(res.wood), money(res.money),
		honor(0), level(0), masterID(0), subMasterID(0)
	{
		dStrcpy(name, sizeof(name), league.name);
	}

	ZoneLeague(const ZoneLeague& zoneLeague)
	{
		*this = zoneLeague;
	}

	ZoneLeague& operator=(const ZoneLeague& zoneLeague)
	{
		memcpy(this, &zoneLeague, sizeof(ZoneLeague));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(id, Base::Bit32);
		stream->writeString(name, _countof(name));
		stream->writeInt(honor, Base::Bit32);
		stream->writeInt(masterOrg, Base::Bit32);
		stream->writeInt(level, Base::Bit8);
		stream->writeInt(subMasterOrg, Base::Bit32);
		stream->writeInt(stone, Base::Bit32);
		stream->writeInt(sprite, Base::Bit32);
		stream->writeInt(wood, Base::Bit32);
		stream->writeInt(money, Base::Bit32);
		stream->writeInt(masterID, Base::Bit32);
		stream->writeInt(subMasterID, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* stream)
	{
		id = stream->readInt(Base::Bit32);
		stream->readString(name, _countof(name));
		honor = stream->readInt(Base::Bit32);
		masterOrg = stream->readInt(Base::Bit32);
		level = stream->readInt(Base::Bit8);
		subMasterOrg = stream->readInt(Base::Bit32);
		stone = stream->readInt(Base::Bit32);
		sprite = stream->readInt(Base::Bit32);
		wood = stream->readInt(Base::Bit32);
		money = stream->readInt(Base::Bit32);
		masterID = stream->readInt(Base::Bit32);
		subMasterID = stream->readInt(Base::Bit32);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* stream) const
	{
		stream->writeInt(change, Base::Bit16);
		if (HasFlag(change, LEAGUE_CHG_Honor))
			stream->writeInt(honor, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_MasterOrg))
			stream->writeInt(masterOrg, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Level))
			stream->writeInt(level, Base::Bit8);
		if (HasFlag(change, LEAGUE_CHG_SubMasterOrg))
			stream->writeInt(subMasterOrg, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Stone))
			stream->writeInt(stone, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Sprite))
			stream->writeInt(sprite, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Wood))
			stream->writeInt(wood, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Money))
			stream->writeInt(money, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_MasterID))
			stream->writeInt(masterID, Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_SubMasterID))
			stream->writeInt(subMasterID, Base::Bit32);
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* stream)
	{
		change = stream->readInt(Base::Bit16);
		if (HasFlag(change, LEAGUE_CHG_Honor))
			honor = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_MasterOrg))
			masterOrg = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Level))
			level = stream->readInt(Base::Bit8);
		if (HasFlag(change, LEAGUE_CHG_SubMasterOrg))
			subMasterOrg = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Stone))
			stone = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Sprite))
			sprite = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Wood))
			wood = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_Money))
			money = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_MasterID))
			masterID = stream->readInt(Base::Bit32);
		if (HasFlag(change, LEAGUE_CHG_SubMasterID))
			subMasterID = stream->readInt(Base::Bit32);
	}
};

struct LeagueApply
{
	int  orgId;
	char orgName[COMMON_STRING_LENGTH];
	int	 order;
	int	 honor;
	int  level;
	int  activity;

	LeagueApply()
	{
		memset(this, 0, sizeof(LeagueApply));
		dStrcpy(orgName, sizeof(orgName), "");
	}

	LeagueApply(const LeagueApply& other)
	{
		*this = other;
	}

	LeagueApply& operator=(const LeagueApply& other)
	{
		memcpy(this, &other, sizeof(LeagueApply));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(orgId, Base::Bit32);
		packet->writeInt(level, Base::Bit8);
		packet->writeString(orgName, _countof(orgName));
		packet->writeInt(order, Base::Bit16);
		packet->writeInt(honor, Base::Bit32);
		packet->writeInt(activity, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		orgId = packet->readInt(Base::Bit32);
		level = packet->readInt(Base::Bit8);
		packet->readString(orgName, _countof(orgName));
		order = packet->readInt(Base::Bit16);
		honor = packet->readInt(Base::Bit32);
		activity = packet->readInt(Base::Bit32);
	}
};

struct LeagueDemesne
{
	int zoneID;
	int leagueID;
	char leagueName[COMMON_STRING_LENGTH];
	DEMESNE_STATE flag;
	int occupyTime;
	int count;
	char masterName[COMMON_STRING_LENGTH];
	int eventTime;

	int applyCount;
	int totalApplyCount;

	LeagueDemesne(int zoneID, int leagueID, DEMESNE_STATE flag, const char* name, const char* master)
		:zoneID(zoneID), leagueID(leagueID), flag(flag), occupyTime(0), count(0), eventTime(0), applyCount(0), totalApplyCount(0)
	{
		dStrcpy(leagueName, sizeof(leagueName), name);
		dStrcpy(masterName, sizeof(masterName), master);
	}

	LeagueDemesne(int zoneID, DEMESNE_STATE flag, int count)
		:zoneID(zoneID), leagueID(0), flag(flag), occupyTime(0), count(count), eventTime(0), applyCount(0), totalApplyCount(0)
	{
		dStrcpy(leagueName, sizeof(leagueName), "");
		dStrcpy(masterName, sizeof(masterName), "");
	}

	LeagueDemesne()
	{
		memset(this, 0, sizeof(LeagueDemesne));
		dStrcpy(leagueName, sizeof(leagueName), "");
		dStrcpy(masterName, sizeof(masterName), "");
	}

	LeagueDemesne(const LeagueDemesne& other)
	{
		*this = other;
	}

	LeagueDemesne& operator=(const LeagueDemesne& other)
	{
		memcpy(this, &other, sizeof(LeagueDemesne));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(zoneID, Base::Bit32);
		packet->writeInt(flag, Base::Bit8);
		switch (flag)
		{
		case DEMESTATE_Apply:
			packet->writeInt(count, Base::Bit16);
			packet->writeInt(applyCount, Base::Bit16);
			packet->writeInt(totalApplyCount, Base::Bit16);
			break;
		case DEMESTATE_Decide:
			packet->writeInt(leagueID, Base::Bit32);
			packet->writeString(leagueName, sizeof(leagueName));
			packet->writeInt(eventTime, Base::Bit32);
			break;
		case DEMESTATE_Occupy:
			packet->writeInt(leagueID, Base::Bit32);
			packet->writeString(leagueName, sizeof(leagueName));
			packet->writeString(masterName, sizeof(masterName));
			break;
		default:
			JUST_ASSERT("Illegal LeagueDemesne flag.");
			break;
		}
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		zoneID = packet->readInt(Base::Bit32);
		flag = (DEMESNE_STATE)packet->readInt(Base::Bit8);
		switch (flag)
		{
		case DEMESTATE_Apply:
			count = packet->readInt(Base::Bit16);
			applyCount = packet->readInt(Base::Bit16);
			totalApplyCount = packet->readInt(Base::Bit16);
			leagueID = 0;
			dStrcpy(leagueName, sizeof(leagueName), "");
			dStrcpy(masterName, sizeof(masterName), "");
			occupyTime = 0;
			eventTime = 0;
			break;
		case DEMESTATE_Decide:
			leagueID = packet->readInt(Base::Bit32);
			packet->readString(leagueName, sizeof(leagueName));
			eventTime = packet->readInt(Base::Bit32);
			dStrcpy(masterName, sizeof(masterName), "");
			count = 0;
			occupyTime = 0;
			break;
		case DEMESTATE_Occupy:
			leagueID = packet->readInt(Base::Bit32);
			packet->readString(leagueName, sizeof(leagueName));
			packet->readString(masterName, sizeof(masterName));
			occupyTime = 0;
			count = 0;
			eventTime = 0;
			break;
		default:
			JUST_ASSERT("Illegal LeagueDemesne flag.");
			break;
		}
	}
};

struct LeagueDemesneIncome
{
	enum LEAGUEDEMESNE_PACK
	{
		LEAGUEDEMESNE_None = 0,
		LEAGUEDEMESNE_Income = BIT(0),
		LEAGUEDEMESNE_Effigy = BIT(1)
	};

	int zoneID;
	int todayWorship;
	int yesterdayWorship;
	int weekShopTax;
	int weekBoothTax;
	int weekStallTax;
	int weekSkillTax;
	int lastWeekShopTax;
	int lastWeekBoothTax;
	int lastWeekStallTax;
	int lastWeekSkillTax;
	int lastWeekWage;
	int effigyID;
	char effigyName[ORG_MAX_EFFGYNAME];
	int applyCount;

	LeagueDemesneIncome()
	{
		memset(this, 0, sizeof(LeagueDemesneIncome));
		dStrcpy(effigyName, sizeof(effigyName), "");
	}

	LeagueDemesneIncome(int zone, int id, const char* name)
	{
		memset(this, 0, sizeof(LeagueDemesneIncome));
		zoneID = zone;
		effigyID = id;
		dStrcpy(effigyName, sizeof(effigyName), name);
	}

	LeagueDemesneIncome(const LeagueDemesneIncome& other)
	{
		*this = other;
	}

	LeagueDemesneIncome& operator=(const LeagueDemesneIncome& other)
	{
		memcpy(this, &other, sizeof(LeagueDemesneIncome));
		return *this;
	}

	void Clear()
	{
		todayWorship = 0;
		yesterdayWorship = 0;
		weekShopTax = 0;
		weekBoothTax = 0;
		weekStallTax = 0;
		weekSkillTax = 0;
		lastWeekShopTax = 0;
		lastWeekBoothTax = 0;
		lastWeekStallTax = 0;
		lastWeekSkillTax = 0;
		lastWeekWage = 0;
	}

	LeagueDemesneIncome& operator+=(const LeagueDemesneIncome& other)
	{
		todayWorship += other.todayWorship;
		yesterdayWorship += other.yesterdayWorship;
		weekShopTax += other.weekShopTax;
		weekBoothTax += other.weekBoothTax;
		weekStallTax += other.weekStallTax;
		weekSkillTax += other.weekSkillTax;
		lastWeekShopTax += other.lastWeekShopTax;
		lastWeekBoothTax += other.lastWeekBoothTax;
		lastWeekStallTax += other.lastWeekStallTax;
		lastWeekSkillTax += other.lastWeekSkillTax;
		lastWeekWage += other.lastWeekWage;
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet, int mode) const
	{
		packet->writeInt(mode, Base::Bit8);
		packet->writeInt(zoneID, Base::Bit32);
		if (HasFlag(mode, LEAGUEDEMESNE_Income))
		{
			packet->writeInt(todayWorship, Base::Bit32);
			packet->writeInt(yesterdayWorship, Base::Bit32);
			packet->writeInt(weekShopTax, Base::Bit32);
			packet->writeInt(weekBoothTax, Base::Bit32);
			packet->writeInt(weekStallTax, Base::Bit32);
			packet->writeInt(weekSkillTax, Base::Bit32);
			packet->writeInt(lastWeekShopTax, Base::Bit32);
			packet->writeInt(lastWeekBoothTax, Base::Bit32);
			packet->writeInt(lastWeekStallTax, Base::Bit32);
			packet->writeInt(lastWeekSkillTax, Base::Bit32);
			packet->writeInt(lastWeekWage, Base::Bit32);
		}
		if (HasFlag(mode, LEAGUEDEMESNE_Effigy))
		{
			packet->writeInt(effigyID, Base::Bit32);
			packet->writeString(effigyName, sizeof(effigyName));
		}
	}

	template<typename _Ty>
	void ReadData(_Ty* packet, int& mode)
	{
		mode = packet->readInt(Base::Bit8);
		zoneID = packet->readInt(Base::Bit32);
		if (HasFlag(mode, LEAGUEDEMESNE_Income))
		{
			todayWorship = packet->readInt(Base::Bit32);
			yesterdayWorship = packet->readInt(Base::Bit32);
			weekShopTax = packet->readInt(Base::Bit32);
			weekBoothTax = packet->readInt(Base::Bit32);
			weekStallTax = packet->readInt(Base::Bit32);
			weekSkillTax = packet->readInt(Base::Bit32);
			lastWeekShopTax = packet->readInt(Base::Bit32);
			lastWeekBoothTax = packet->readInt(Base::Bit32);
			lastWeekStallTax = packet->readInt(Base::Bit32);
			lastWeekSkillTax = packet->readInt(Base::Bit32);
			lastWeekWage = packet->readInt(Base::Bit32);
		}
		if (HasFlag(mode, LEAGUEDEMESNE_Effigy))
		{
			effigyID = packet->readInt(Base::Bit32);
			packet->readString(effigyName, sizeof(effigyName));
		}
	}

	void ReadUpdate(const LeagueDemesneIncome& other, int mode)
	{
		zoneID = other.zoneID;
		if (HasFlag(mode, LEAGUEDEMESNE_Income))
		{
			todayWorship = other.todayWorship;
			yesterdayWorship = other.yesterdayWorship;
			weekShopTax = other.weekShopTax;
			weekBoothTax = other.weekShopTax;
			weekStallTax = other.weekShopTax;
			weekSkillTax = other.weekSkillTax;
			lastWeekShopTax = other.lastWeekShopTax;
			lastWeekBoothTax = other.lastWeekBoothTax;
			lastWeekStallTax = other.lastWeekStallTax;
			lastWeekSkillTax = other.lastWeekSkillTax;
			lastWeekWage = other.lastWeekWage;
		}
		if (HasFlag(mode, LEAGUEDEMESNE_Effigy))
		{
			effigyID = other.effigyID;
			dStrcpy(effigyName, sizeof(effigyName), other.effigyName);
		}
	}
};

struct ZoneClientOrg
{
	enum ORG_CHANG
	{
		ORG_CHG_NULL = 0,
		ORG_CHG_OrgID = BIT(0),
		ORG_CHG_OrgName = BIT(1),
		ORG_CHG_MemberMoney = BIT(2),
		ORG_CHG_AwardMoney = BIT(3),
		ORG_CHG_MemberLeagueMoney = BIT(4),
		ORG_CHG_MemberContribute = BIT(5),
		ORG_CHG_OrgLevel = BIT(6),
		ORG_CHG_OrgMaster = BIT(7),
		ORG_CHG_OrgSubMaster = BIT(8),
		ORG_CHG_TradeRight = BIT(9),
		ORG_CHG_PutOutMission = BIT(10),
		ORG_CHG_ORGFULL = BIT(11) - 1,

		ORG_CHG_LeagueID = BIT(11),
		ORG_CHG_LeagueName = BIT(12),
		ORG_CHG_LeagueMasterOrg = BIT(13),
		ORG_CHG_LeagueLevel = BIT(14),
		ORG_CHG_LeagueSubMasterOrg = BIT(15),
		ORG_CHG_LEAGUEFULL = (BIT(16) - 1) & (~ORG_CHG_ORGFULL),

		ORG_CHG_FULL = 0xFFFF
	};

	int id;
	char name[COMMON_STRING_LENGTH];
	int money;
	int	awardMoney;
	int	awardCanTradeMoney;
	int leagueMoney;
	int contribute;
	int orgLevel;
	int orgMaster;
	bool orgSubMaster;
	bool tradeRight;
	int putOutMissionTime[Org::_maxPutOutMissionNum];//发布任务时间

	int leagueID;
	char leagueName[COMMON_STRING_LENGTH];
	int leagueMasterOrg;
	int leagueLevel;
	int leagueSubMasterOrg;

	ZoneClientOrg()
	{
		memset(this, 0, sizeof(ZoneClientOrg));
		dStrcpy(name, sizeof(name), "");
		dStrcpy(leagueName, sizeof(leagueName), "");
	}

	ZoneClientOrg(const Org& org, const OrgMember& orgMember)
		:id(org.id), money(orgMember.money), leagueMoney(orgMember.leagueMoney), contribute(orgMember.contribute), orgLevel(org.level),
		orgMaster(org.masterId), orgSubMaster(orgMember.subMaster), tradeRight(orgMember.tradeRight),
		leagueID(0), leagueMasterOrg(0), leagueSubMasterOrg(0), leagueLevel(0), awardMoney(orgMember.awardMoney),
		awardCanTradeMoney(orgMember.awardCanTradeMoney)
	{
		dStrcpy(name, sizeof(name), org.name);
		dStrcpy(leagueName, sizeof(leagueName), "");
		for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
			putOutMissionTime[i] = org.putOutMissionTime[i];
	}

	ZoneClientOrg(const ZoneClientOrg& org)
	{
		*this = org;
	}

	ZoneClientOrg& operator=(const ZoneClientOrg& org)
	{
		memcpy(this, &org, sizeof(ZoneClientOrg));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(id, Base::Bit32);
		stream->writeString(name, _countof(name));
		stream->writeInt(money, Base::Bit32);
		stream->writeInt(awardMoney, Base::Bit32);
		stream->writeInt(awardCanTradeMoney, Base::Bit32);
		stream->writeInt(leagueMoney, Base::Bit32);
		stream->writeInt(contribute, Base::Bit32);
		stream->writeInt(orgLevel, Base::Bit8);
		stream->writeInt(orgMaster, Base::Bit32);
		stream->writeFlag(orgSubMaster);
		stream->writeFlag(tradeRight);
		for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
			stream->writeInt(putOutMissionTime[i], Base::Bit32);

		stream->writeInt(leagueID, Base::Bit32);
		stream->writeString(leagueName, _countof(leagueName));
		stream->writeInt(leagueMasterOrg, Base::Bit32);
		stream->writeInt(leagueLevel, Base::Bit8);
		stream->writeInt(leagueSubMasterOrg, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* stream)
	{
		id = stream->readInt(Base::Bit32);
		stream->readString(name, _countof(name));
		money = stream->readInt(Base::Bit32);
		awardMoney = stream->readInt(Base::Bit32);
		awardCanTradeMoney = stream->readInt(Base::Bit32);
		leagueMoney = stream->readInt(Base::Bit32);
		contribute = stream->readInt(Base::Bit32);
		orgLevel = stream->readInt(Base::Bit8);
		orgMaster = stream->readInt(Base::Bit32);
		orgSubMaster = stream->readFlag();
		tradeRight = stream->readFlag();
		for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
			putOutMissionTime[i] = stream->readInt(Base::Bit32);

		leagueID = stream->readInt(Base::Bit32);
		stream->readString(leagueName, _countof(leagueName));
		leagueMasterOrg = stream->readInt(Base::Bit32);
		leagueLevel = stream->readInt(Base::Bit8);
		leagueSubMasterOrg = stream->readInt(Base::Bit32);
	}

	template<typename _Ty>
	void WriteUpdate(int change, _Ty* stream) const
	{
		stream->writeInt(change, Base::Bit16);
		if (HasFlag(change, ORG_CHG_OrgID))
			stream->writeInt(id, Base::Bit32);
		if (HasFlag(change, ORG_CHG_OrgName))
			stream->writeString(name, _countof(name));
		if (HasFlag(change, ORG_CHG_MemberMoney))
			stream->writeInt(money, Base::Bit32);
		if (HasFlag(change, ORG_CHG_AwardMoney))
		{
			stream->writeInt(money, Base::Bit32);
			stream->writeInt(awardMoney, Base::Bit32);
			stream->writeInt(awardCanTradeMoney, Base::Bit32);
		}
		if (HasFlag(change, ORG_CHG_MemberLeagueMoney))
			stream->writeInt(leagueMoney, Base::Bit32);
		if (HasFlag(change, ORG_CHG_MemberContribute))
			stream->writeInt(contribute, Base::Bit32);
		if (HasFlag(change, ORG_CHG_OrgLevel))
			stream->writeInt(orgLevel, Base::Bit8);
		if (HasFlag(change, ORG_CHG_OrgMaster))
			stream->writeInt(orgMaster, Base::Bit32);
		if (HasFlag(change, ORG_CHG_OrgSubMaster))
			stream->writeFlag(orgSubMaster);
		if (HasFlag(change, ORG_CHG_TradeRight))
			stream->writeFlag(tradeRight);
		if (HasFlag(change, ORG_CHG_PutOutMission))
			for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
				stream->writeInt(putOutMissionTime[i], Base::Bit32);

		if (HasFlag(change, ORG_CHG_LeagueID))
			stream->writeInt(leagueID, Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueName))
			stream->writeString(leagueName, _countof(leagueName));
		if (HasFlag(change, ORG_CHG_LeagueMasterOrg))
			stream->writeInt(leagueMasterOrg, Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueLevel))
			stream->writeInt(leagueLevel, Base::Bit8);
		if (HasFlag(change, ORG_CHG_LeagueSubMasterOrg))
			stream->writeInt(leagueSubMasterOrg, Base::Bit32);
	}

	template<typename _Ty>
	void ReadUpdate(int& change, _Ty* stream)
	{
		change = stream->readInt(Base::Bit16);
		if (HasFlag(change, ORG_CHG_OrgID))
			id = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_OrgName))
			stream->readString(name, _countof(name));
		if (HasFlag(change, ORG_CHG_MemberMoney))
			money = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_AwardMoney))
		{
			money = stream->readInt(Base::Bit32);
			awardMoney = stream->readInt(Base::Bit32);
			awardCanTradeMoney = stream->readInt(Base::Bit32);
		}
		if (HasFlag(change, ORG_CHG_MemberLeagueMoney))
			leagueMoney = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_MemberContribute))
			contribute = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_OrgLevel))
			orgLevel = stream->readInt(Base::Bit8);
		if (HasFlag(change, ORG_CHG_OrgMaster))
			orgMaster = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_OrgSubMaster))
			orgSubMaster = stream->readFlag();
		if (HasFlag(change, ORG_CHG_TradeRight))
			tradeRight = stream->readFlag();
		if (HasFlag(change, ORG_CHG_PutOutMission))
			for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
				putOutMissionTime[i] = stream->readInt(Base::Bit32);

		if (HasFlag(change, ORG_CHG_LeagueID))
			leagueID = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueName))
			stream->readString(leagueName, _countof(leagueName));
		if (HasFlag(change, ORG_CHG_LeagueMasterOrg))
			leagueMasterOrg = stream->readInt(Base::Bit32);
		if (HasFlag(change, ORG_CHG_LeagueLevel))
			leagueLevel = stream->readInt(Base::Bit8);
		if (HasFlag(change, ORG_CHG_LeagueSubMasterOrg))
			leagueSubMasterOrg = stream->readInt(Base::Bit32);
	}

	int ReadUpdate(int change, const ZoneOrg& zoneOrg)
	{
		int clientchange = ORG_CHG_NULL;
		if (HasFlag(change, ZoneOrg::ORG_CHG_Master))
		{
			orgMaster = zoneOrg.master;
			clientchange |= ORG_CHG_OrgMaster;
		}
		if (HasFlag(change, ZoneOrg::ORG_CHG_Level))
		{
			orgLevel = zoneOrg.level;
			clientchange |= ORG_CHG_OrgLevel;
		}
		if (HasFlag(change, ZoneOrg::ORG_CHG_LeagueID) && zoneOrg.leagueID == 0)
		{
			leagueID = 0;
			dStrcpy(leagueName, _countof(leagueName), "");
			leagueMasterOrg = 0;
			leagueLevel = 0;
			leagueSubMasterOrg = 0;
			clientchange |= ORG_CHG_LEAGUEFULL;
		}
		if (HasFlag(change, ZoneOrg::ORG_CHG_AwardMoney))
		{
			money = zoneOrg.money;
			clientchange |= ORG_CHG_AwardMoney;
		}
		if (HasFlag(change, ZoneOrg::ORG_CHG_PutOutMission))
		{
			for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
				putOutMissionTime[i] = zoneOrg.putOutMissionTime[i];
			clientchange |= ORG_CHG_PutOutMission;
		}
		return clientchange;
	}

	int ReadUpdate(int change, const ZoneLeague& zoneLeague)
	{
		int clientchange = ORG_CHG_NULL;
		if (HasFlag(change, ZoneLeague::LEAGUE_CHG_MasterOrg))
		{
			leagueMasterOrg = zoneLeague.masterOrg;
			clientchange |= ORG_CHG_LeagueMasterOrg;
		}
		if (HasFlag(change, ZoneLeague::LEAGUE_CHG_Level))
		{
			leagueLevel = zoneLeague.level;
			clientchange |= ORG_CHG_LeagueLevel;
		}
		if (HasFlag(change, ZoneLeague::LEAGUE_CHG_SubMasterOrg))
		{
			leagueSubMasterOrg = zoneLeague.subMasterOrg;
			clientchange |= ORG_CHG_LeagueSubMasterOrg;
		}
		return clientchange;
	}

	int ReadUpdate(const ZoneLeague& zoneLeague)
	{
		leagueID = zoneLeague.id;
		dStrcpy(leagueName, _countof(leagueName), zoneLeague.name);
		leagueMasterOrg = zoneLeague.masterOrg;
		leagueLevel = zoneLeague.level;
		leagueSubMasterOrg = zoneLeague.subMasterOrg;
		return ORG_CHG_LEAGUEFULL;
	}

	void ReadUpdate(int change, const ZoneClientOrg& org)
	{
		if (HasFlag(change, ORG_CHG_OrgID))
			id = org.id;
		if (HasFlag(change, ORG_CHG_OrgName))
			dStrcpy(name, _countof(name), org.name);
		if (HasFlag(change, ORG_CHG_MemberMoney))
			money = org.money;
		if (HasFlag(change, ORG_CHG_AwardMoney))
		{
			money = org.money;
			awardMoney = org.awardMoney;
			awardCanTradeMoney = org.awardCanTradeMoney;
		}
		if (HasFlag(change, ORG_CHG_MemberLeagueMoney))
			leagueMoney = org.leagueMoney;
		if (HasFlag(change, ORG_CHG_MemberContribute))
			contribute = org.contribute;
		if (HasFlag(change, ORG_CHG_OrgLevel))
			orgLevel = org.orgLevel;
		if (HasFlag(change, ORG_CHG_OrgMaster))
			orgMaster = org.orgMaster;
		if (HasFlag(change, ORG_CHG_OrgSubMaster))
			orgSubMaster = org.orgSubMaster;
		if (HasFlag(change, ORG_CHG_TradeRight))
			tradeRight = org.tradeRight;
		if (HasFlag(change, ORG_CHG_PutOutMission))
			for (int i = 0; i < Org::_maxPutOutMissionNum; ++i)
				putOutMissionTime[i] = org.putOutMissionTime[i];

		if (HasFlag(change, ORG_CHG_LeagueID))
			leagueID = org.leagueID;
		if (HasFlag(change, ORG_CHG_LeagueName))
			dStrcpy(leagueName, _countof(leagueName), org.leagueName);
		if (HasFlag(change, ORG_CHG_LeagueMasterOrg))
			leagueMasterOrg = org.leagueMasterOrg;
		if (HasFlag(change, ORG_CHG_LeagueLevel))
			leagueLevel = org.leagueLevel;
		if (HasFlag(change, ORG_CHG_LeagueSubMasterOrg))
			leagueSubMasterOrg = org.leagueSubMasterOrg;
	}
};

struct LeagueDB
{
	struct LeagueData
	{
		League league;
		LeagueRes res;
	};
	struct ApplyData
	{
		int leagueID;
		int orgID;
	};
	typedef std::vector<LeagueData> Leagues;
	typedef Leagues::iterator LeaguesIter;
	typedef std::vector<ApplyData> Applys;
	typedef Applys::iterator ApplysIter;
	typedef std::vector<LeagueDemesne> Demesnes;
	typedef Demesnes::iterator DemesnesIter;
	typedef std::vector<LeagueDemesneIncome> Incomes;
	typedef Incomes::iterator IncomesIter;

	Leagues leagues;
	Applys applys;
	Demesnes demesnes;
	Incomes incomes;
};

struct LeagueCreate
{
	League league;
	OrgIDSet creator;
};

struct UpdateInfo
{
	UpdateInfo(int change, OrgOp op = ORGOP_NULL, int playerID = 0, bool bUpdateDB = true)
		:change(change), op(op), playerID(playerID), bUpdateDB(bUpdateDB)
	{
	}
	int change;
	OrgOp op;
	bool bUpdateDB;
	int playerID;
};

#include <map>

struct OrgBuf
{
	int honor;
	int activity;

	OrgBuf()
	{
		memset(this, 0, sizeof(OrgBuf));
	}

	OrgBuf(const OrgBuf& orgBuf)
	{
		*this = orgBuf;
	}

	OrgBuf& operator=(const OrgBuf& orgBuf)
	{
		memcpy(this, &orgBuf, sizeof(OrgBuf));
		return *this;
	}

	OrgBuf& operator+=(const OrgBuf& orgBuf)
	{
		honor += orgBuf.honor;
		activity += orgBuf.activity;
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(OrgBuf) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(OrgBuf) << 3, this);
	}
};

typedef std::map<int, OrgBuf> OrgBufs;
typedef OrgBufs::iterator OrgBufsIter;

struct OrgMemberBuf
{
	int honor;
	int trade;
	int mission;

	OrgMemberBuf()
	{
		memset(this, 0, sizeof(OrgMemberBuf));
	}

	OrgMemberBuf(const OrgMemberBuf& orgMemberBuf)
	{
		*this = orgMemberBuf;
	}

	OrgMemberBuf& operator=(const OrgMemberBuf& orgMemberBuf)
	{
		memcpy(this, &orgMemberBuf, sizeof(OrgMemberBuf));
		return *this;
	}

	OrgMemberBuf& operator+=(const OrgMemberBuf& orgMemberBuf)
	{
		honor += orgMemberBuf.honor;
		trade += orgMemberBuf.trade;
		mission += orgMemberBuf.mission;
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(OrgMemberBuf) << 3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(OrgMemberBuf) << 3, this);
	}
};

typedef std::map<int, OrgMemberBuf> OrgMemberBufs;
typedef OrgMemberBufs::iterator OrgMemberBufsIter;

typedef std::vector<int> OrgIDs;
typedef OrgIDs::const_iterator OrgIDsIter;

enum ORGMODIFY_TYPE
{
	ORGMODIFY_ClearCallBossTime,
	ORGMODIFY_ClearSetMasterTime,
	ORGMODIFY_ClearTransferTime,
	LEAGUEMODIFY_ClearTransferTime,
	ORGMODIFY_ClearLeaveTime,
	LEAGUEMODIFY_ClearLeaveTime
};

enum ORGNOTIFY_TYPE
{
	ORGNOTIFY_DemesneDefend,
	ORGNOTIFY_DemesneOccupy,
	ORGNOTIFY_DemesneDecide,
	ORGNOTIFY_DemesneDecideLost,

	LEAGUENOTIFY_DemesneDefend,
	LEAGUENOTIFY_DemesneOccupy,
	LEAGUENOTIFY_DemesneDecide,
	LEAGUENOTIFY_DemesneDecideLost
};

#endif
