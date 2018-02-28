#ifndef __VISIT_BASE_H
#define __VISIT_BASE_H
#include "BASE/types.h"
#include "BASE/bitStream.h"
#include <string>
#include <memory>

struct stCity
{
	S32 PlayerID;
	S16 CityID;
	S8  Level;
	S16	Support;
	S8	DevelopFlag;

	stCity() : PlayerID(0), CityID(0), Level(0), Support(0), DevelopFlag(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(CityID,	Base::Bit16);
		stream->writeInt(Level,		Base::Bit8);
		stream->writeInt(Support,	Base::Bit16);
		stream->writeInt(DevelopFlag, Base::Bit8);
	}
};

typedef std::tr1::shared_ptr<stCity>	 CityRef;
#endif //__SKILL_BASE_H