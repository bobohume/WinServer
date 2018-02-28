#ifndef __ACTIVITYBASE_H
#define __ACTIVITYBASE_H
#include "BASE/types.h"
#include "BASE/bitStream.h"
#include <string>
#include <memory>
#include "../Common/UtilString.h"

#define MAX_ACTIVITY_NUM 4
struct stActivity
{
	S32 ID;
	S32 Type;
	std::string Name;
	std::string Contend;
	S32 BeginTime;
	S32	EndTime;
	S32 ItemId[MAX_ACTIVITY_NUM];
	S32 ItemNum[MAX_ACTIVITY_NUM];
	S32 Extend[MAX_ACTIVITY_NUM];//贪官0:资源上限,1：单此资源,2:资源类型  3:广播间隔
								 //排行榜0:开始区间，1：结束区间,3:广播间隔
								 //消费活动0:开始区间，1：结束区间,2:自增量用于领取标志
	S32 CreateTime;
	//no db
	S32 NotifyTime;

	stActivity() : ID(0), Type(0), BeginTime(0), EndTime(0), NotifyTime(0), Name(""), Contend(""), CreateTime(0)
	{
		memset(ItemId, 0, sizeof(ItemId));
		memset(ItemNum, 0, sizeof(ItemNum));
		memset(Extend, 0, sizeof(Extend));
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(ID,				Base::Bit32);
		stream->writeInt(Type,				Base::Bit32);
		stream->writeString(Util::MbcsToUtf8(Name));
		stream->writeString(Util::MbcsToUtf8(Contend));
		stream->writeInt(BeginTime,			Base::Bit32);
		stream->writeInt(EndTime,			Base::Bit32);

		stream->writeInt(MAX_ACTIVITY_NUM, Base::Bit16);
		for (int i = 0; i < MAX_ACTIVITY_NUM; ++i)
		{
			if (stream->writeFlag(ItemId[i]))
			{
				stream->writeInt(ItemId[i],  Base::Bit32);
			}
		}

		stream->writeInt(MAX_ACTIVITY_NUM, Base::Bit16);
		for (int i = 0; i < MAX_ACTIVITY_NUM; ++i)
		{
			if (stream->writeFlag(ItemNum[i]))
			{
				stream->writeInt(ItemNum[i], Base::Bit32);
			}
		}

		stream->writeInt(MAX_ACTIVITY_NUM, Base::Bit16);
		for (int i = 0; i < MAX_ACTIVITY_NUM; ++i)
		{
			if (stream->writeFlag(Extend[i]))
			{
				stream->writeInt(Extend[i], Base::Bit32);
			}
		}
	}

	void WriteDataSimple(Base::BitStream* stream) {
		stream->writeInt(Type, Base::Bit32);
		stream->writeInt(BeginTime, Base::Bit32);
		stream->writeInt(EndTime, Base::Bit32);
	}
};

typedef std::tr1::shared_ptr<stActivity>	ActivityRef;
#endif //__ACTIVITYBASE_H