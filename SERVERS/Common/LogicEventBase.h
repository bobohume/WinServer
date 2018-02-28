#ifndef __LOGICEVENTBASE_H__
#define __LOGICEVENTBASE_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"

struct stEventItem
{
	enum
	{
		RESERVE_MAX			= 3,			//保留数据个数

		EVENTSTATE_NONE		= 0,			//未处理
		EVENTSTATE_SEND		= 1,			//已发送

		EVENTTYPE_NONE		= 0,			//非定时事件
		EVENTTYPE_TIMER		= 1,			//定时事件

		DATATYPE_NONE		= 0,			//无
		DATATYPE_MONEY		= 1,			//金元
		DATATYPE_BINDMONEY	= 2,			//绑定金元
		DATATYPE_GOLD		= 3,			//元宝
		DATATYPE_BINDGOLD	= 4,			//绑定元宝
		DATATYPE_FAME		= 5,			//声望
		DATATYPE_BANKMONEY	= 6,			//银行金元
		DATATYPE_EXP		= 7,			//经验
		DATATYPE_EXPPOOL	= 8,			//师傅经验池经验
		DATATYPE_EXPLOIT	= 9,			//师德
		DATATYPE_REDUCEGOLD	= 10,			//扣减元宝


		REASON_OK			= 0,			//处理成功
		REASON_SEND			= 1,			//查询并发送
		REASON_OVERTIME		= 2,			//超时定时时间
		REASON_INVALIDTIME	= 3,			//事件过期
	};

	U64		id;								//唯一ID
	U32		sender;							//发送人
	U32		recver;							//收取人
	U8		event_state;					//事件状态(见EVENTSTATE枚举值)
	U8		event_type;						//事件类型(见EVENTTYPE枚举值)
	U32		timer;							//若非定时事件,则此值为过期时间
											//若定时事件,则此值为定时时间
	U8		data_type;						//收取数据类型(见DATATYPE)
	S32		data_value;						//收取数据
	S32		reserve[RESERVE_MAX];			//保留值
	stEventItem():id(0),sender(0),recver(0),
		event_type(EVENTTYPE_NONE),timer(1602864000),
		data_type(DATATYPE_NONE),data_value(0),
		event_state(EVENTSTATE_NONE)
	{
		for(int i = 0; i < RESERVE_MAX; i++)
			reserve[i] = 0;
	}

	template<typename T>
	void PackData(T* stream)
	{
		stream->writeBits(Base::Bit64, &id);
		stream->writeInt(sender,		Base::Bit32);
		stream->writeInt(recver,		Base::Bit32);
		stream->writeInt(event_state,	Base::Bit8);
		stream->writeInt(event_type,	Base::Bit8);
		stream->writeInt(timer,			Base::Bit32);
		stream->writeInt(data_type,		Base::Bit8);
		stream->writeInt(data_value,	Base::Bit32);
		for(int i = 0; i < RESERVE_MAX; i++)
			stream->writeInt(reserve[i],Base::Bit32);
	}

	template<typename T>
	void UnpackData(T* stream)
	{
		stream->readBits(Base::Bit64, &id);
		sender		= stream->readInt(Base::Bit32);
		recver		= stream->readInt(Base::Bit32);
		event_state	= stream->readInt(Base::Bit8);
		event_type	= stream->readInt(Base::Bit8);
		timer		= stream->readInt(Base::Bit32);
		data_type	= stream->readInt(Base::Bit8);
		data_value	= stream->readInt(Base::Bit32);
		for(int i = 0; i < RESERVE_MAX; i++)
			reserve[i] = stream->readInt(Base::Bit32);
	}
};

#endif//__LOGICEVENTBASE_H__