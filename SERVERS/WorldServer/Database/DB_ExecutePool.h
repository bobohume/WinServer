#ifndef __DBEXECUTEPOOL_H__
#define __DBEXECUTEPOOL_H__

#include "base/types.h"
#include "AsyncThread/Http_Async.h"

class Async_Execution;
class DB_Execution;
class DBThreadManager;
class DataBase;

class DB_ExecutePool
{
public:
	DB_ExecutePool(DataBase* pDatabase,int threadCount);
	~DB_ExecutePool(void);

	int Post(DB_Execution* pExecution);
	int Post(Async_Execution* pExecution);

	DB_Execution* Create();
	Async_Execution* CreateAsyncPacket();

	int GetUID();
	static void Export(struct lua_State* L);
protected:
	static int m_id;
	DBThreadManager* m_pManager;
};

#endif //__DBEXECUTEPOOL_H__