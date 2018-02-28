#ifndef GLOBAL_EVENT_BASE_H
#define GLOBAL_EVENT_BASE_H

#include "PacketType.h"
#include <vector>

#define EVENT_DATA2_LENGTH 250

struct GlobalEvent
{
	int eventID;
	int eventData1;
	char eventData2[EVENT_DATA2_LENGTH];

	GlobalEvent()
	{
		memset(this, 0, sizeof(GlobalEvent));
	}

	GlobalEvent(const GlobalEvent& other)
	{
		*this = other;
	}

	GlobalEvent& operator=(const GlobalEvent& other)
	{
		memcpy(this, &other, sizeof(GlobalEvent));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(eventID, Base::Bit32);
		packet->writeInt(eventData1, Base::Bit32);
		packet->writeString(eventData2, sizeof(eventData2));
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		eventID = packet->readInt(Base::Bit32);
		eventData1 = packet->readInt(Base::Bit32);
		packet->readString(eventData2, sizeof(eventData2));
	}
};

typedef std::vector<GlobalEvent> GlobalEvents;
typedef GlobalEvents::iterator GoabalEventsIter;

#endif