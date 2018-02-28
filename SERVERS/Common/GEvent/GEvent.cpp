#include "base/bitStream.h"
#include "GEvent.h"

GEvent::GEvent()
{
	ver=EVENT_VER;
	uid=-1;
	sqid=0;
	mType = invalidEvent;
};

bool GEvent::packData(void *bitStream)
{
	Base::BitStream *pStream = (Base::BitStream *)bitStream;

	pStream->writeBits(sizeof(mEventData)<<3,&mEventData);
	return true;
}

bool GEvent::unpackData(void *bitStream,int ver)
{
	Base::BitStream *pStream = (Base::BitStream *)bitStream;

	pStream->readBits(sizeof(mEventData)<<3,&mEventData);
	return true;
}
