#ifndef __BUFF_BASE_H
#define __BUFF_BASE_H
#include "BASE/types.h"
#include "Common/PlayerStruct.h"
#include <string>
#include <memory>

struct stSkill
{
	S32 PlayerID;
	S32	Skill;

	stSkill() : PlayerID(0), Skill(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(PlayerID, Base::Bit32);
		stream->writeInt(Skill, Base::Bit32);
	}
};

typedef std::tr1::shared_ptr<stSkill> SkillRef;
#endif //__BUFF_BASE_H