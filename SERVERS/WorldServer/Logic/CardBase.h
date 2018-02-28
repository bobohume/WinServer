#ifndef CARD_BASE_H
#define CARD_BASE_H
#include "BASE/types.h"
#include "Common/PlayerStruct.h"
#include <hash_map>
#include <string>
#include <memory>

const unsigned int MAX_EQUIP_SLOT = 3;
const unsigned int MAX_SKILL_SLOT = 9;
const unsigned int MAX_ADD_SLOT   = 4;

enum CARD_MASK
{
	CARD_MASK_NULL = 0,
	CARD_MASK_SKILL	  = BIT(0),
	CARD_MASK_ADD_AGI = BIT(1),
	CARD_MASK_ADD_INT = BIT(2),
	CARD_MASK_ADD_STR = BIT(3),
};

struct stSimCard
{
	std::string UID;
	S32		Agi;
	S32		Int;
	S32		Str;

	stSimCard() : UID(""), Agi(0), Int(0), Str(0)
	{
	}

	template<class T>
	stSimCard(const T obj) : UID(obj->UID), Agi(obj->Agi), Int(obj->Int), Str(obj->Str)
	{
	}
};


//卡牌结构
struct stCard
{
	std::string UID;//唯一ID
	U32		PlayerID;
	U32		CardID;
	U16		Level;
	U32		Achievement;
	U8		OfficialRank;//官品
	U8		OfficialPos;//爵位ID
	U32		CardMask;

	U32		Agi;
	U32		Int;
	U32		Str;

	U16		Agi_Grow;
	U16		Int_Grow;
	U16		Str_Grow;

	std::string Equip[MAX_EQUIP_SLOT];//装备ID

	U32		Skill[MAX_SKILL_SLOT];
	U16		Agi_Add[MAX_ADD_SLOT];
	U16		Int_Add[MAX_ADD_SLOT];
	U16		Str_Add[MAX_ADD_SLOT];
	S32		BattleVal;
	U64		SkillsExFailedNum;/* 1000000 7bit * 9*/
	U32		AptExFailedNum;/* 1000000 7bit * 3 */

	stCard():CardID(0), PlayerID(0), Level(0), Achievement(0), OfficialRank(0), OfficialPos(0),\
		CardMask(0), Agi(0), Int(0), Str(0), UID(""), Agi_Grow(0), Int_Grow(0), Str_Grow(0), \
		BattleVal(0), SkillsExFailedNum(0), AptExFailedNum(0)
	{
		memset(Skill,	0, sizeof(Skill));
		memset(Agi_Add, 0, sizeof(Agi_Add));
		memset(Int_Add, 0, sizeof(Int_Add));
		memset(Str_Add, 0, sizeof(Str_Add));
	}

	~stCard()
	{

	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeString(UID, MAX_UUID_LENGTH);
		stream->writeInt(CardID, Base::Bit32);
		stream->writeInt(Level, Base::Bit16);
		stream->writeInt(Achievement, Base::Bit32);
		stream->writeInt(OfficialRank, Base::Bit8);
		stream->writeInt(OfficialPos, Base::Bit8);

		stream->writeInt(Agi, Base::Bit32);
		stream->writeInt(Int, Base::Bit32);
		stream->writeInt(Str, Base::Bit32);

		stream->writeInt(Agi_Grow, Base::Bit16);
		stream->writeInt(Int_Grow, Base::Bit16);
		stream->writeInt(Str_Grow, Base::Bit16);

		stream->writeInt(MAX_EQUIP_SLOT, Base::Bit16);
		for (int i = 0; i < MAX_EQUIP_SLOT; ++i)
		{
			stream->writeFlag(true);
			stream->writeString(Equip[i], MAX_UUID_LENGTH);
		}

		stream->writeInt(MAX_SKILL_SLOT, Base::Bit16);
		for (int i = 0; i < MAX_SKILL_SLOT; ++i)
		{
			if (stream->writeFlag(Skill[i]))
			{
				stream->writeInt(Skill[i], Base::Bit32);
			}
		}

		stream->writeInt(MAX_ADD_SLOT, Base::Bit16);
		for (int i = 0; i < MAX_ADD_SLOT; ++i)
		{
			stream->writeFlag(true);
			stream->writeInt(Agi_Add[i], Base::Bit16);
		}

		stream->writeInt(MAX_ADD_SLOT, Base::Bit16);
		for (int i = 0; i < MAX_ADD_SLOT; ++i)
		{
			stream->writeFlag(true);
			stream->writeInt(Int_Add[i], Base::Bit16);
		}

		stream->writeInt(MAX_ADD_SLOT, Base::Bit16);
		for (int i = 0; i < MAX_ADD_SLOT; ++i)
		{
			stream->writeFlag(true);
			stream->writeInt(Str_Add[i], Base::Bit16);
		}

		stream->writeBits(Base::Bit64, &SkillsExFailedNum);

		stream->writeInt(AptExFailedNum, Base::Bit32);
	}
};

typedef std::tr1::shared_ptr<stCard> CardRef;
#endif //CARD_BASE_H