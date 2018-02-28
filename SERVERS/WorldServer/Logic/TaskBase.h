#ifndef TASK_BASE_H
#define TASK_BASE_H
#include "Common/PlayerStruct.h"
#include "BASE/types.h"
#include "BASE/base64.h"
#include <string>

struct stTaskInfo
{
	S32 TaskId;
	U32	PlayerID;
	S32 TaskVal;
	S32 FinishFlag;

	stTaskInfo() : TaskId(0), TaskVal(0), FinishFlag(0), PlayerID(0)
	{
	}

	stTaskInfo(S32 TaskId, S32 TaskVal, S32 FinishFlag, S32 PlayerID)
		:TaskId(TaskId), TaskVal(TaskVal), FinishFlag(FinishFlag), PlayerID(PlayerID)
	{}

	~stTaskInfo() {}


	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(TaskId, Base::Bit32);
		stream->writeInt(TaskVal, Base::Bit32);
		stream->writeInt(FinishFlag, Base::Bit32);
	}
};

typedef std::shared_ptr<stTaskInfo> TaskInfoRef;
#endif //TASK_BASE_H