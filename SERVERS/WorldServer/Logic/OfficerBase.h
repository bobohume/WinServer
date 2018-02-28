#ifndef OFFICER_BASE_H
#define OFFICER_BASE_H

#include "Base/types.h"
#include "Base/bitStream.h"
#include "Common/PlayerStruct.h"
#include <memory>
#include <string>

struct stOfficer
{
	U8   OfficeID;
	U32	 PlayerID;
	std::string Card;
	S32  Business;
	S32  Military;
	S32  Technology;

	stOfficer():OfficeID(0), PlayerID(0), Card(""),\
		Business(0), Military(0), Technology(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(OfficeID, Base::Bit8);
		stream->writeString(Card, MAX_UUID_LENGTH);
	}
};

typedef std::tr1::shared_ptr<stOfficer> OfficerRef;
#endif//OFFICER_BASE_H
