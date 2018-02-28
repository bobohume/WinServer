#ifndef __BOSS_BASE_H
#define __BOSS_BASE_H
#include <memory>
#include "BASE/types.h"
#include "BASE/bitStream.h"

#define MAX_BATTLE_NUM 3
#define MAX_BUFF_NUM 2

struct stBoss
{
	S32 BossId;
	S32 DataId;
	S64 Hp;
	S32 Type;
	S32 CreateTime;
	S32 EndTime;

	stBoss() : BossId(0), Hp(0), CreateTime(0), EndTime(0), Type(0), DataId(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(BossId, Base::Bit32);
		stream->writeInt(DataId, Base::Bit32);
		stream->writeInt(Type, Base::Bit32);
		stream->writeBits(sizeof(S64) << 3, &Hp);
		stream->writeInt(CreateTime, Base::Bit32);
		stream->writeInt(EndTime, Base::Bit32);
	}
};

struct stCardBattleInfo
{
	std::string UID;
	S32 Agi;
	S32 Int;
	S32 Str;
	S32 Agi_Grow;
	S32 Int_Grow;
	S32 Str_Grow;


	stCardBattleInfo():Agi(0), Int(0), Str(0), Agi_Grow(0),\
		Int_Grow(0), Str_Grow(0), UID("")
	{
	}

	template<class T>
	stCardBattleInfo& operator = (const T pCard) {
		this->UID = pCard->UID;
		this->Agi = pCard->Agi;
		this->Int = pCard->Int;
		this->Str = pCard->Str;
		this->Agi_Grow = pCard->Agi_Grow;
		this->Int_Grow = pCard->Int_Grow;
		this->Str_Grow = pCard->Str_Grow;
		return *this;
	}
};

struct stBuffInfo
{
	S32 Add_gPc;
	S32 CoolDown;

	stBuffInfo() :Add_gPc(0.0f), CoolDown(0) {
	}
};

struct stBossBattleInfo
{
	S32 BossId;
	S32 PlayerId;
	stCardBattleInfo m_Card[MAX_BATTLE_NUM];
	stBuffInfo	m_Buff[MAX_BUFF_NUM];
	S32 CurPos;
	S32 CoolDown;

	stBossBattleInfo() : PlayerId(0), CurPos(0), CoolDown(0), BossId(0)
	{
		memset(m_Card, 0, sizeof(m_Card));
		memset(m_Buff, 0, sizeof(m_Buff));
	}

	template<class T>
	stBossBattleInfo& operator = (const T other) {
		this->BossId = other->BossId;
		this->PlayerId = other->PlayerId;
		this->CurPos = other->CurPos;
		this->CoolDown = other->CoolDown;
		for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
			this->m_Card[i].Agi = other->Agi[i];
			this->m_Card[i].Int = other->Int[i];
			this->m_Card[i].Str = other->Str[i];
			this->m_Card[i].Agi_Grow = other->Agi_Grow[i];
			this->m_Card[i].Int_Grow = other->Int_Grow[i];
			this->m_Card[i].Str_Grow = other->Str_Grow[i];
		}
		for (auto i = 0; i < MAX_BUFF_NUM; ++i) {
			this->m_Buff[i].Add_gPc = other->Add_gPc[i];
			this->m_Buff[i].CoolDown = other->Add_CoolDown[i];
		}
		return *this;
	}
};

struct stBattleRank
{
	S32 BossId;
	S32 PlayerId;
	S64 Hp;
	std::string PlayerName;

	stBattleRank() : PlayerId(0), Hp(0), BossId(0)
	{	
	}
};

struct stBattleInfoDB 
{
	S32 BossId;
	S32 PlayerId;
	S32 CurPos;
	std::string UID[MAX_BATTLE_NUM];
	S32 Agi[MAX_BATTLE_NUM];
	S32 Agi_Grow[MAX_BATTLE_NUM];
	S32 Int[MAX_BATTLE_NUM];
	S32 Int_Grow[MAX_BATTLE_NUM];
	S32 Str[MAX_BATTLE_NUM];
	S32 Str_Grow[MAX_BATTLE_NUM];
	S32 Add_gPc[MAX_BUFF_NUM];
	S32 Add_CoolDown[MAX_BUFF_NUM];
	S32 CoolDown;

	stBattleInfoDB()
	{
		memset(this, 0, sizeof(stBattleInfoDB));
	}

	stBattleInfoDB& operator = (const stBossBattleInfo* other) {
		this->BossId = other->BossId;
		this->PlayerId = other->PlayerId;
		this->CurPos = other->CurPos;
		this->CoolDown = other->CoolDown;
		for (auto i = 0; i < MAX_BATTLE_NUM; ++i) {
			this->UID[i] = other->m_Card[i].UID;
			this->Agi[i] = other->m_Card[i].Agi;
			this->Int[i] = other->m_Card[i].Int;
			this->Str[i] = other->m_Card[i].Str;
			this->Agi_Grow[i] = other->m_Card[i].Agi_Grow;
			this->Int_Grow[i] = other->m_Card[i].Int_Grow;
			this->Str_Grow[i] = other->m_Card[i].Str_Grow;
		}
		for (auto i = 0; i < MAX_BUFF_NUM; ++i) {
			this->Add_gPc[i] = other->m_Buff[i].Add_gPc;
			this->Add_CoolDown[i] = other->m_Buff[i].CoolDown;
		}
		return *this;
	}
};

typedef std::tr1::shared_ptr<stBoss>			BossRef;
typedef std::tr1::shared_ptr<stBossBattleInfo>	BossBattleInfoRef;
typedef std::tr1::shared_ptr<stBattleRank>		BossBattleRankRef;
#endif//__BOSS_BASE_H