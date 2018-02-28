#if !defined(TCP_SERVER)
#include "core/bitStream.h"
#endif

#include "Common/PlayerStructEx.h"

void stItemInfo::WriteData(Base::BitStream* stream)
{
	stream->writeString(UID, MAX_UUID_LENGTH);
	stream->writeInt(ItemID, Base::Bit32);
	stream->writeInt(Quantity, Base::Bit16);
	stream->writeInt(Level, Base::Bit8);
	stream->writeInt(Exp, Base::Bit32);
}

void stItemInfo::ReadData(Base::BitStream* stream)
{
	stream->readString(UID, MAX_UUID_LENGTH);
	ItemID = stream->readInt(Base::Bit32);
	Quantity = stream->readInt(Base::Bit16);
	Level = stream->readInt(Base::Bit8);
	Exp = stream->readInt(Base::Bit32);
}