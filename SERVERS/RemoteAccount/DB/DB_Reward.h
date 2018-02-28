#ifndef __DB_REWARD_H__
#define __DB_REWARD_H__

#include "CommLib/ThreadPool.h"
#include "Common/RewardBase.h"

// ----------------------------------------------------------------------------
// ’ ∫≈Ω±¿¯µƒ¡Ï»°
class DB_Reward_Draw : public ThreadBase
{
public:
	DB_Reward_Draw(void) : UID("") 
	{
	}
	~DB_Reward_Draw(void) {}
	virtual int Execute(int ctxId,void* param);
	std::string UID;
};
#endif //__DB_REWARD_H__