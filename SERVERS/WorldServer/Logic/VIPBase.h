#pragma once

#include "Base/types.h"
#include "Base/bitStream.h"
#include "Common/PlayerStruct.h"
#include <memory>
#include <string>

struct stVIPBase 
{
	U32 VipLevel;
	U32 DrawGold;

	stVIPBase() : VipLevel(0), DrawGold(0) {}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(VipLevel, Base::Bit32);
		stream->writeInt(DrawGold, Base::Bit32);
	}
};