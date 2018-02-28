#ifndef GLOBAL_EVENT_H
#define GLOBAL_EVENT_H
#include "base/types.h"

//·ÏÆúµÄ
struct stGlobalEvent
{
	unsigned char id;
	int			time;
	char		type;

	static const int MaxEventCount = 256;
};


#define  MAX_EVENT 255

struct stEventData 
{
	stEventData(void)
	{
		id = -1;
		Time = 0;
		Type = 0;
		LineId = 0;
	}

	S32 id;
	U64 Time;
	U32 Type;
	U32 LineId;
};

#endif