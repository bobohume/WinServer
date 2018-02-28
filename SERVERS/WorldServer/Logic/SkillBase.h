#ifndef __SKILL_BASE_H
#define __SKILL_BASE_H
#include "BASE/types.h"
#include "Common/PlayerStruct.h"
#include <string>
#include <memory>

struct stSkill
{
	S32 PlayerID;
	S32 Series;
	S32 Level;

	stSkill() : PlayerID(0), Series(0), Level(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(Series, Base::Bit32);
		stream->writeInt(Level, Base::Bit32);
	}
};

typedef std::tr1::shared_ptr<stSkill>	 SkillRef;
#endif //__SKILL_BASE_H