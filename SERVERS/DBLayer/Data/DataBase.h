#ifndef __DATABASE_H__
#define __DATABASE_H__

#ifndef __DBERROR_H__
#include "DBError.h"
#endif

class CDBConnPool;
class CMyLog;

class DataBase
{
public:
	DataBase();
	~DataBase();
	DBError Initialize(const char *svr, const char *db, const char *usr, const char *pwd, int maxconn = 5);
	CDBConnPool* GetPool() { return mDBConnPool;}
	CMyLog* GetLog();
	void ShutDown();
private:
	CDBConnPool* mDBConnPool;
};

#endif//__DBMANAGE_H__