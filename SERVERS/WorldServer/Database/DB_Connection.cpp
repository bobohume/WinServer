
#include "stdafx.h"
#include "db_Connection.h"
#include "DB_ExecutePool.h"
#include "DBLayer/Data/DataBase.h"
#include "base/log.h"
#include "Script/lua_tinker.h"

DB_Connection::DB_Connection(void)
{
	m_pDatabase = 0;
}

DB_Connection::~DB_Connection(void)
{
	SAFE_DELETE(m_pDatabase);
}

bool DB_Connection::Init(const char *svr, const char *db, const char *usr, const char *pwd, int maxconn)
{
	SAFE_DELETE(m_pDatabase);

	m_pDatabase = new DataBase;

	if (0 == m_pDatabase)
		return false;

	DBError err = m_pDatabase->Initialize(svr,db,usr,pwd,maxconn);

	if(err != DBERR_NONE)
	{
		g_Log.WriteFocus("Warning:: %s数据库初始化失败!",db);
		return false;
	}

	return true;
}

DB_ExecutePool* DB_Connection::Create(int threadCount)
{
	if (0 == m_pDatabase)
	{
		assert(0);
		return 0;
	}

	DB_ExecutePool* pool = new DB_ExecutePool(m_pDatabase,threadCount);
	return pool;
}

void DB_Connection::Export(struct lua_State* L)
{
	lua_tinker::class_add<DB_Connection>(L, "DateBase");
	lua_tinker::class_con<DB_Connection>(L, lua_tinker::constructor<DB_Connection>);

	lua_tinker::class_def<DB_Connection>(L, "createPool",&DB_Connection::Create);
	lua_tinker::class_def<DB_Connection>(L, "init",&DB_Connection::Init);
}