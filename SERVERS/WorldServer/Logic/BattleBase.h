#ifndef BATTLE_BASE_H
#define BATTLE_BASE_H
#include "BASE/types.h"
#include <string>
#include <memory>

//中间属性，不需要存数据库
struct stBattleBase
{
	S64		Hp;
	S64		Attack;
	S32		Critical;
	S32		Dodge;
	S32		Troops;

	F32		Hp_Per;
	F32		Atk_Per;

	S32		SoldierHp;
	S32		SoldierAtk;

	inline	void CaculateTroops()
	{
		Troops = (S32)ceil(Hp / (SoldierHp *  (1.0 + Hp_Per)));
	}
	inline	void CaculateHp()
	{
		Hp = ceil((S64)Troops * SoldierHp * (1.0 + Hp_Per));
	}
	inline	void CaculateAttack()
	{
		Attack = ceil((S64)Troops * SoldierAtk * (1.0 + Atk_Per));
	}

	stBattleBase() : Hp(0), Attack(0), Critical(0), Dodge(0), Troops(0), \
		Hp_Per(0.0f), Atk_Per(0.0f), SoldierHp(0), SoldierAtk(0)
	{
	}

	void Clear()
	{
		memset(this, 0, sizeof(stBattleBase));
	}

	stBattleBase(S32 iSoildierHp, S32 iSoildierAtk, S32 iTroops) : Hp(((S64)iSoildierHp)*iTroops), Attack(((S64)iSoildierAtk)*iTroops), \
		Critical(0), Dodge(0), Troops(iTroops), Hp_Per(0.0f), Atk_Per(0.0f), SoldierHp(iSoildierHp), SoldierAtk(iSoildierAtk)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		//stream->writeBits(sizeof(S64) << 3, &Hp);
		//stream->writeBits(sizeof(S64) << 3, &Attack);
		//stream->writeBits(sizeof(S64) << 3, &Troops);
		stream->writeInt(Troops, Base::Bit32);
	}
};

struct stBattleInfo : public stBattleBase
{
	std::string		Card[4];
	U8				CardNum;
	U32				GateId;

	stBattleInfo() : stBattleBase(), CardNum(0), GateId(70010001)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(GateId,	Base::Bit32);
		stream->writeBits(sizeof(S64) << 3, &Troops);
		stream->writeInt(SoldierHp, Base::Bit32);
		stream->writeInt(SoldierAtk,Base::Bit32);
		stream->writeInt(Critical,  Base::Bit32);
	}
};

typedef std::tr1::shared_ptr<stBattleInfo> BattleInfoRef;
#endif //BATTLE_BASE_H