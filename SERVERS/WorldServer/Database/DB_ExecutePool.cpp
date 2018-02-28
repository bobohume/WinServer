
#include "stdafx.h"
#include "DB_ExecutePool.h"
#include "DB_Execution.h"
#include "AsyncThread/Http_Async.h"
#include "CommLib/DBThreadManager.h"
#include "WorldServer.h"
#include "DBLib/dbPool.h"
#include "DBLayer/Data/DataBase.h"
#include "DBContext.h"
#include "Script/lua_tinker.h"

int DB_ExecutePool::m_id = 1;

DB_ExecutePool::DB_ExecutePool(DataBase* pDatabase,int threadCount)
{
	if (threadCount < 0)
	{
		threadCount = 1;
	}

	m_pManager = new DBThreadManager;
	assert(0 != m_pManager);
	m_pManager->Start(pDatabase->GetPool(),threadCount,DB_CONTEXT_NOMRAL);
}

DB_ExecutePool::~DB_ExecutePool(void)
{
	if (0 != m_pManager)
	{
		m_pManager->Stop();
	}

	SAFE_DELETE(m_pManager);
}

DB_Execution* DB_ExecutePool::Create()
{
	return new DB_Execution(this);
}

Async_Execution* DB_ExecutePool::CreateAsyncPacket()
{
	return new Async_Execution(this);
}

int DB_ExecutePool::Post(DB_Execution* pExecution)
{
	assert(0 != m_pManager);

	if (0 == pExecution->m_id)
	{
		m_id++;

		if (m_id < 0)
		{
			m_id = 1;
		}

		pExecution->SetId(m_id);
	}

	m_pManager->SendPacket(pExecution);
	return pExecution->m_id;
}

int DB_ExecutePool::Post(Async_Execution* pAsyncExecution)
{
	assert(0 != m_pManager);

	if (0 == pAsyncExecution->m_id)
	{
		m_id++;

		if (m_id < 0)
		{
			m_id = 1;
		}

		pAsyncExecution->SetId(m_id);
	}

	m_pManager->SendPacket(pAsyncExecution);
	return pAsyncExecution->m_id;
}

int DB_ExecutePool::GetUID()
{
	m_id++;

	if (m_id < 0)
	{
		m_id = 1;
	}

	return m_id;
}

void DB_ExecutePool::Export(struct lua_State* L)
{
	lua_tinker::class_add<DB_ExecutePool>(L, "DBPool");
	lua_tinker::class_def<DB_ExecutePool>(L, "create",&DB_ExecutePool::Create);
	lua_tinker::class_def<DB_ExecutePool>(L, "createAsync",&DB_ExecutePool::CreateAsyncPacket);
	lua_tinker::class_def<DB_ExecutePool>(L, "uid",&DB_ExecutePool::GetUID);
}
