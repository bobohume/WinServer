#ifndef __TBLBASE_H__
#define __TBLBASE_H__

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#include "base/Locker.h"
#include "base/log.h"

class CDBConn;
class CMyLog;

class TBLBase
{
public:
	TBLBase(DataBase* db);
	TBLBase(CDBConn* conn);

	virtual ~TBLBase();
	DataBase* GetDB() { return mDataBase;}
	void	 AttachConn(CDBConn* conn);
	CDBConn* GetConn();
protected:
	bool	 m_isNeedDetach;

	CDBConn* mDBConn;	//注:一个数据库连接当次只能处理一个数据操作,不能在当次数据操作的
						//结果集被处理完前,又中途去处理另一个数据操作.例如下面作法违例:
						//M_SQL(GetConn(),"SELECT Accountid FROM Tbl_Account");
						//while(GetConn()->More())
						//{
						//   M_SQL(GetConn(),"SELECT id FROM Tbl_Player WHERE accountid=%d", GetConn()->GetInt());
						//   while(GetConn()->More())
						//   {
						//		//.....
						//	 }
						//}
	DataBase* mDataBase;
};

//做数据库操作的相关统计,把所有代码里面的GetConn->SQL转换为自定义的宏,
//并记录操作时的文件和函数名
class DBTraceNode;

class DBStat
{
    struct StatNode
    {
        StatNode(void)
        {
            pNode = 0;
            count = 0;
        }

        DBTraceNode* pNode;
        volatile long count;
    };
public:
    enum {MAXDBNODEID = 10000};

    static DBStat* Instance(void)
    {
        static DBStat local;
        return &local;
    }

    DBStat(void)
    {
        m_id = 1;
    }

    void Post(DBTraceNode* pNode);
    void Dump(void);

    int  GetId(void);
    
    CMyCriticalSection m_cs;

    int      m_id;
    StatNode m_nodes[MAXDBNODEID];
};

class DBTraceNode
{
public:
    DBTraceNode(const char* file,const char* func,int line)
    {
        m_fileName = file ? file : "unknown";

        int pos = m_fileName.rfind("\\");

        if (pos >= 0)
        {
            m_fileName = m_fileName.substr(pos + 1,m_fileName.length());
        }

        m_funcName = func ? func : "unknown";
        m_line     = line;
        m_id       = DBStat::Instance()->GetId();
    }

    void Post(void)
    {
        if (!ms_isEnabled)
            return;

#ifndef GM_CLIENT
        //g_Log.WriteLog("db call[%s] line[%d]",m_funcName.c_str(),m_line);
        DBStat::Instance()->Post(this);
#endif
    }

    LONG m_id;
    int  m_line;
    std::string m_fileName;
    std::string m_funcName;

    static bool ms_isEnabled;
};

#ifndef DISABLE_DB_STAT
    #define M_SQL(caller,...) {static DBTraceNode dbnode(__FILE__,__FUNCTION__,__LINE__);dbnode.Post();} caller->SQL(__VA_ARGS__);
#else
    #define M_SQL(caller,...) caller->SQL(__VA_ARGS__);
#endif

#endif//__TBLBASE_H__
