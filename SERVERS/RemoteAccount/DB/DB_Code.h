#ifndef __DB_CODE_H__
#define __DB_CODE_H__

#include "CommLib/ThreadPool.h"
#include "Common/RewardBase.h"

// ----------------------------------------------------------------------------
// ’ ∫≈Ω±¿¯µƒ¡Ï»°
class DB_Code_Draw : public ThreadBase
{
public:
	DB_Code_Draw(void) : UID("")
	{
	}
	~DB_Code_Draw(void) {}
	virtual int Execute(int ctxId,void* param);
	std::string UID;
};
#endif //__DB_CODE_H__