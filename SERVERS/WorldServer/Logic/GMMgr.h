#ifndef GM_MANAGER_H
#define GM_MANAGER_H

#include <hash_map>
#include "WINTCP/IPacket.h"
#include "BaseMgr.h"
#include "CommLib/redis.h"
#include "CommLib/ThreadProcedure.h"

class GMCmdProcedure : public ThreadProcedure
{
public:
	GMCmdProcedure();

	~GMCmdProcedure(void)
	{
		m_isEnded = true;
	}

	void Stop(void) { m_isEnded = true; }

	//method from ThreadProcedure
	virtual int run(void);
private:
	bool m_isEnded;
};
#endif//GM_MANAGER_H
