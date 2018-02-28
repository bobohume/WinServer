#include "Define.h"
#include "dblib/dbLib.h"
#include "Base/Log.h"
#include "TBLBase.h"
#include <assert.h>

bool DBTraceNode::ms_isEnabled = true;

TBLBase::TBLBase(DataBase* db)
{
	mDataBase = db;
	CDBConnPool* mPool = NULL;
	if(db)
		mPool = db->GetPool();

	if(mPool)
	{
		mDBConn = mPool->Lock();
		m_isNeedDetach = true;
	}
	else
    {
		mDBConn = NULL;
        m_isNeedDetach = false;
    }
}

TBLBase::~TBLBase()
{
	if(mDBConn && m_isNeedDetach)
	{
		mDBConn->Detach();
		mDBConn = 0;
	}
}

void TBLBase::AttachConn(CDBConn* conn)
{
	if (0 == conn)
	{
		assert(0 != conn);
		return;
	}
	
	assert(0 == mDBConn);
	mDBConn = conn;
}

CDBConn* TBLBase::GetConn()
{
	if(mDBConn == NULL)
	{
		throw CDBException(-1, -1, -1,"数据库连接池中获取连接失败!","","TBLBase::GetConn()",0);
	}

	return mDBConn;
}

int DBStat::GetId(void)
{
    OLD_DO_LOCK(m_cs);

    return m_id++;
}

void DBStat::Post(DBTraceNode* pNode)
{
    if (0 == pNode || pNode->m_id >= MAXDBNODEID)
    {
        return;
    }
    
    if (0 == m_nodes[pNode->m_id].pNode)
    {
        m_nodes[pNode->m_id].pNode = pNode;
    }

    InterlockedIncrement(&m_nodes[pNode->m_id].count);
}

void DBStat::Dump(void)
{
    try
    {
        g_Log.WriteLog("db--------------------------------------------------------------------");

        for (int i = 0; i < MAXDBNODEID; ++i)
        {
            if (0 == m_nodes[i].pNode || 0 == m_nodes[i].count)
            {
                continue;
            }

            g_Log.WriteLog("db stat[%s]:[%s]:line[%d] freq[%d]",m_nodes[i].pNode->m_fileName.c_str(),
                m_nodes[i].pNode->m_funcName.c_str(),m_nodes[i].pNode->m_line,m_nodes[i].count);
            
            InterlockedExchange(&m_nodes[i].count,0);
        }
    }
    catch(...)
    {
    }
}