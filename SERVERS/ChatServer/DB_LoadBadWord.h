#ifndef _DBLOADBADWORD_
#define _DBLOADBADWORD_

#include "CommLib/ThreadPool.h"
#include "../WorldServer/ChatMgr.h"
#include "DBLayer/Data/TBLBadWord.h"

class DB_LoadBadWord : public ThreadBase
{
public:
    DB_LoadBadWord(void);

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
   
	std::vector<std::string> m_badWordList;
};

#endif 