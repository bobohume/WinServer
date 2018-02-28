#ifndef _MultiMountBase_H
#define _MultiMountBase_H

#if defined(NTJ_SERVER) || defined(NTJ_CLIENT) || defined(NTJ_EDITOR)
#include "platform/types.h"
#else
#include "base/bitStream.h"
#endif

//多人骑乘的相关信息
struct stMTMountGroup
{
	enum
	{
		Invaild_Slot = -1,
		Master_Slot = 0,
		Max_Member = 6,

		MTMount_Buff = 320220001,
	};
	stMTMountGroup()
	{
		memset(this,0,sizeof(stMTMountGroup));
	}
	unsigned int uId;
	int nMaxSlot;
	int member[Max_Member];

	template<typename _Ty>
	void read(_Ty* stream)
	{
		uId = stream->readInt(32 );
		nMaxSlot = stream->readInt(32);
		for (int i = 0 ; i < Max_Member; ++i)
		{
			member[i] = stream->readInt(32 );
		}	
	}
	template<typename _Ty>
	void write(_Ty* stream)
	{
		stream->writeInt( uId, 32 );
		stream->writeInt(nMaxSlot,32);
		for (int i = 0 ; i < Max_Member; ++i)
		{
			stream->writeInt( member[i], 32 );
		}
	}
};
#endif