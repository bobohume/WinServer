/*
#ifndef _UNCOMPRESSTHREAD_
#define _UNCOMPRESSTHREAD_

#include "CommLib/ThreadPool.h"
#include "Common/PlayerStructEx.h"

class UncompressThread : public ThreadBase
{
public:
    UncompressThread(void);
	~UncompressThread();

	//method from ThreadBase
	virtual int Execute(int ctxId,void* param);
    
	int m_tarPlayerId;
	int m_srcPlayerId;
	int m_socketHandle;
	char m_buff[MAX_PLAYER_STRUCT_SIZE];
	int m_size;
	stPlayerStruct m_Player;
    int m_rank;

};
*/
#endif /*_UNCOMPRESSTHREAD_*/