#ifndef _DBCONNECTION_H_
#define _DBCONNECTION_H_

#include "base/types.h"

class DataBase;
class DB_ExecutePool;

class DB_Connection
{
public:
	DB_Connection(void);
	~DB_Connection(void);

	DB_ExecutePool* Create(int threadCount);
	bool Init(const char *svr, const char *db, const char *usr, const char *pwd, int maxconn);

	static void Export(struct lua_State* L);

protected:
	DataBase* m_pDatabase;
};

#endif /*_DBCONNECTION_H_*/