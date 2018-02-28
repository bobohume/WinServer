#ifndef __PRINCESS_BASE_H
#define __PRINCESS_BASE_H
#include "BASE/types.h"
#include "Common/UtilString.h"
#include "Common/PlayerStruct.h"
#include <string>
#include <memory>

struct stPrincess
{
	std::string UID;
	S32 PlayerID;
	S32	PrincessID;
	S32 Support;//宠幸
	S32 SupportTime;
	S32 SupportTimes;
	S32 Charm;//魅力
	S32 Rite;//礼仪
	S32 Culture;//修养值
	S32 Battle;//宫斗值
	S32 PrincessPos;//妃位
	S32 Skill[3];
	U8  PrinceNum;
	U8  PrinceAbility;//皇子能力提升
	S32 BattleVal;

	// NO DB
	S32 CharmEx;

	stPrincess() : UID(""), PrincessID(0), Support(0), Charm(0), Rite(0),\
		Culture(0), Battle(0), PrincessPos(0), PrinceNum(0), SupportTime(0), \
		SupportTimes(0), BattleVal(0), PrinceAbility(0), CharmEx(0)
	{
		memset(Skill, 0, sizeof(Skill));
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeString(UID, MAX_UUID_LENGTH);
		stream->writeInt(PrincessID, Base::Bit32);
		stream->writeInt(Support, Base::Bit32);
		stream->writeInt(SupportTime, Base::Bit32);
		stream->writeInt(SupportTimes, Base::Bit32);
		stream->writeInt(CharmEx, Base::Bit32);
		stream->writeInt(Rite, Base::Bit32);
		stream->writeInt(Culture, Base::Bit32);
		stream->writeInt(Battle, Base::Bit32);
		stream->writeInt(PrincessPos, Base::Bit32);

		stream->writeInt(3, Base::Bit16);
		for (auto i = 0; i < 3; ++i)
		{
			if (stream->writeFlag(Skill[i]))
			{
				stream->writeInt(Skill[i], Base::Bit32);
			}
		}

		stream->writeInt(PrinceNum, Base::Bit8);
	}
};

struct stPrince
{
	std::string UID;
	S32 PlayerID;
	std::string PrincessUID;
	std::string CardUID;
	std::string Name;
	U8	Quatily;
	S32 Level;
	S32 Exp;
	S32 Agi;
	S32 Int;
	S32 Str;
	S32 OpenTime;
	S8	OpenFlag;
	S8  Peerages;
	S8  Activity;
	S32 ActivityTime;

	// NO DB
	S32 AgiEx;
	S32 IntEx;
	S32 StrEx;

	stPrince() : UID(""), PrincessUID(""), CardUID(""), Name(""), Quatily(0), \
		Level(1), Exp(0), Agi(0), Int(0), Str(0), Peerages(0), Activity(0),	  \
		OpenTime(0), OpenFlag(0), AgiEx(0), IntEx(0), StrEx(0), ActivityTime(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeString(UID, MAX_UUID_LENGTH);
		stream->writeString(PrincessUID, MAX_UUID_LENGTH);
		stream->writeString(CardUID, MAX_UUID_LENGTH);
		stream->writeString(Util::MbcsToUtf8(Name), 50);
		stream->writeInt(Quatily, Base::Bit8);
		stream->writeInt(Level, Base::Bit32);
		stream->writeInt(Exp, Base::Bit32);
		stream->writeInt(AgiEx, Base::Bit32);
		stream->writeInt(IntEx, Base::Bit32);
		stream->writeInt(StrEx, Base::Bit32);
		stream->writeInt(OpenTime, Base::Bit32);
		stream->writeInt(Peerages, Base::Bit8);
		stream->writeInt(Activity, Base::Bit8);
		stream->writeInt(OpenFlag, Base::Bit8);
		stream->writeInt(ActivityTime, Base::Bit32);
	}
};

typedef std::tr1::shared_ptr<stPrincess> PrincessRef;
typedef std::tr1::shared_ptr<stPrince>	 PrinceRef;
#endif //__PRINCESS_BASE_H