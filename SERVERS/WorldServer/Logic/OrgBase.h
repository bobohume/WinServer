#pragma once
#include <string>
#include <memory>
#include "BASE/types.h"
#include "Common/PacketType.h"

struct stOrg
{
	S32     Id;
	S32		Level;
	S32		Exp;
	S32		SkillPoint;//科技点
	std::string Name;
	std::string	Qq;
	std::string Msg;
	std::string	Memo;
	S32		MasterId;
	S32		TransferTime;
	S32		BattleVal;
	S16		PlayerNum;
	S16		MaxPlayerNum;
	S16		MaxEliteNum;
	S8		AutoAcceptJoin;
	S8		OrgBossTimes;
	S32		OrgBossTime;

	stOrg()
		:Id(0), Level(1), Exp(0), SkillPoint(0), Name(""), Qq(""), Msg(""), Memo(""), MasterId(0)
		, AutoAcceptJoin(false), TransferTime(0), BattleVal(0), PlayerNum(0), MaxPlayerNum(10),
		MaxEliteNum(0), OrgBossTimes(0), OrgBossTime(0)
	{
	}

	stOrg(const stOrg& other)
	{
		*this = other;
	}

	stOrg& operator=(const stOrg& other)
	{
		memcpy(this, &other, sizeof(stOrg));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(Id, Base::Bit32);
		stream->writeInt(Level, Base::Bit8);
		stream->writeInt(Exp, Base::Bit32);
		stream->writeInt(SkillPoint, Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(Name));
		stream->writeString(Util::MbcsToUtf8(Qq));
		stream->writeString(Util::MbcsToUtf8(Msg));
		stream->writeString(Util::MbcsToUtf8(Memo));
		stream->writeInt(MasterId, Base::Bit32);
		stream->writeFlag(AutoAcceptJoin);
		stream->writeInt(TransferTime, Base::Bit32);
		stream->writeInt(BattleVal, Base::Bit32);
		stream->writeInt(PlayerNum, Base::Bit32);
		stream->writeInt(MaxPlayerNum, Base::Bit32);
		stream->writeInt(MaxEliteNum, Base::Bit32);
		if (time(NULL) >= OrgBossTime)
		{
			stream->writeInt(0, Base::Bit8);
		}
		else 
		{
			stream->writeInt(OrgBossTimes, Base::Bit8);
		}
		stream->writeInt(OrgBossTime, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* stream)
	{
		Id = stream->readInt(Base::Bit32);
		Level = stream->readInt(Base::Bit8);
		Exp = stream->readInt(Base::Bit32);
		SkillPoint = stream->readInt(Base::Bit32);
		Name = stream->readString();
		Qq = stream->readString();
		Msg = stream->readString();
		Memo = stream->readString();
		MasterId = stream->readInt(Base::Bit32);
		AutoAcceptJoin = stream->readFlag();
		TransferTime = stream->readInt(Base::Bit32);
		BattleVal = stream->readInt(Base::Bit32);
		PlayerNum = stream->readInt(Base::Bit32);
		MaxPlayerNum = stream->readInt(Base::Bit32);
		MaxEliteNum = stream->readInt(Base::Bit32);
		OrgBossTimes = stream->readInt(Base::Bit8);
		OrgBossTime = stream->readInt(Base::Bit32);
	}
};

//成员
struct stOrgMember
{
	S32 OrgId;
	S32 PlayerId;
	std::string PlayerName;
	S32 Contribute;
	S32 GiftExp;//成员宝箱等级
	S32 MemberLevel;//成员等级
	S32 MemberStatus;//成员状态，离开帮会
	S32 BattleVal;//战斗力
	S32 Vip;
	S32 PlayerFace;
	S32 LeaveTime;

	stOrgMember() : OrgId(0), PlayerId(0), PlayerName(""), Contribute(0),\
		MemberLevel(0), MemberStatus(0), LeaveTime(0), BattleVal(0), GiftExp(0), \
		Vip(0), PlayerFace(0)
	{
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(PlayerId, Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(PlayerName));
		stream->writeInt(Contribute, Base::Bit32);
		stream->writeInt(GiftExp, Base::Bit32);
		stream->writeInt(MemberLevel, Base::Bit32);
		stream->writeInt(MemberStatus, Base::Bit32);
		stream->writeInt(BattleVal, Base::Bit32);
		stream->writeInt(Vip, Base::Bit32);
		stream->writeInt(PlayerFace, Base::Bit32);
		stream->writeInt(LeaveTime, Base::Bit32);
	}
};

//科技
struct stOrgSkill
{
	S32 OrgId;
	S32 SkillId;
	S16 Level;

	stOrgSkill() : OrgId(0), SkillId(0), Level(0)
	{
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(SkillId, Base::Bit32);
		stream->writeInt(Level, Base::Bit16);
	}
};

//申请
struct stOrgApply
{
	S32 OrgId;
	S32 PlayerId;
	std::string PlayerName;
	S32 BattleVal;
	S32 ApplyTime;
	S32 Vip;
	S32 PlayerFace;
	S8	OnlineFlag;


	stOrgApply() : OrgId(0), PlayerId(0), PlayerName(""), BattleVal(0), ApplyTime(0), \
		PlayerFace(0), Vip(0), OnlineFlag(0)
	{
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(PlayerId, Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(PlayerName));
		stream->writeInt(BattleVal, Base::Bit32);
		stream->writeInt(ApplyTime, Base::Bit32);
		stream->writeInt(Vip,		Base::Bit32);
		stream->writeInt(PlayerFace,Base::Bit32);
		stream->writeInt(OnlineFlag, Base::Bit8);
	}
};

//日志
struct stOrgLog 
{
	enum enOrgLog
	{
		ORG_LOG_NONE,
		ORG_LOG_JOIN,//加入联盟
		ORG_LOG_APPLY,//申请加入联盟
		ORG_LOG_AGREEAPPLY,//同意加入
		ORG_LOG_REFUSEAPPLY,//拒绝加入
		ORG_LOG_LEAVE,//离开联盟
		ORG_LOG_KICK,//踢出联盟
		ORG_LOG_DONATE,//捐献
		ORG_LOG_MEMBERLEAVE,//成员等级
		ORG_LOG_TRANSFER,//转移盟主
		ORG_LOG_SKILL,//技能升级
		ORG_LOG_BUYITEM,//购买物品
		ORG_LOG_BUYGIFT,//购买宝箱
	};

	S32			OrgId;
	S32			CreateTime;
	std::string SourceName;
	std::string TargetName;
	S32			OptType;
	S32			Val0;
	S32			Val1;

	stOrgLog() : OrgId(0), SourceName(""), TargetName(""), OptType(0),  Val0(0),
		Val1(0), CreateTime(0)
	{
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(CreateTime, Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(SourceName));
		stream->writeString(Util::MbcsToUtf8(TargetName));
		stream->writeInt(OptType, Base::Bit32);
		stream->writeInt(Val0, Base::Bit32);
		stream->writeInt(Val1, Base::Bit8);
	}
};

struct stSimpleOrg
{
	S32     Id;
	S32		Level;
	std::string Name;
	std::string MasterName;
	S32		MasterId;
	S32		BattleVal;
	S16		PlayerNum;
	S16		MaxPlayerNum;
	S8		AutoAcceptJoin;
	std::string Memo;

	stSimpleOrg() : Id(0), Level(0), Name(""), MasterName(""), MasterId(0), BattleVal(0),
		PlayerNum(0), MaxPlayerNum(0), AutoAcceptJoin(0), Memo("")
	{
	}

	stSimpleOrg(stOrg* other);

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeInt(Id, Base::Bit32);
		stream->writeInt(Level, Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(Name));
		stream->writeString(Util::MbcsToUtf8(MasterName));
		stream->writeInt(MasterId, Base::Bit32);
		stream->writeInt(BattleVal, Base::Bit32);
		stream->writeInt(PlayerNum, Base::Bit16);
		stream->writeInt(MaxPlayerNum, Base::Bit16);
		stream->writeFlag(AutoAcceptJoin);
		stream->writeString(Util::MbcsToUtf8(Memo));
	}
};

struct stOrgGift
{
	S32 OrgId;
	S32 PlayerId;
	std::string UID;
	S32 GiftId;
	std::string PlayerName;

	stOrgGift() : OrgId(0), GiftId(0), UID(""), PlayerId(0), PlayerName("")
	{
	}

	template<typename _Ty>
	void WriteData(_Ty* stream) const
	{
		stream->writeString(UID);
		stream->writeInt(GiftId, Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(PlayerName));
	}
};


typedef std::tr1::shared_ptr<stOrgMember>	OrgMemberRef;
typedef std::tr1::shared_ptr<stOrgSkill>	OrgSkillRef;
typedef std::tr1::shared_ptr<stOrgApply>	OrgApplyRef;
typedef std::tr1::shared_ptr<stOrgLog>		OrgLogRef;
typedef std::tr1::shared_ptr<stOrgGift>		OrgGiftRef;