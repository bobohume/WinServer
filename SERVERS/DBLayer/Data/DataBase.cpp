#include "Define.h"
#include "DataBase.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"

DataBase::DataBase()
{
	mDBConnPool = NULL;
}

DataBase::~DataBase()
{
	ShutDown();
}

// ----------------------------------------------------------------------------
// 初始化数据连接
DBError DataBase::Initialize(const char *svr, const char *db, const char *usr, const char *pwd, int maxconn /* = 5 */)
{
	mDBConnPool = new CDBConnPool;
	if(mDBConnPool->Open(maxconn, svr, usr, pwd, db))
	{
		return DBERR_NONE;
	}
	delete mDBConnPool;
	mDBConnPool = NULL;
	return DBERR_DB_INITIALIZE;
}

void DataBase::ShutDown()
{
	if(mDBConnPool)
	{
		mDBConnPool->Close();
		SAFE_DELETE(mDBConnPool)
	}
}

CMyLog* DataBase::GetLog()
{
	return &g_Log;
}