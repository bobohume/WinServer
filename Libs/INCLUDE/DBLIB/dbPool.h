
#pragma once
#ifndef _DBPOOL_H
#define _DBPOOL_H

#include "dbCore.h"
#include "dbException.h"
#include "Base\Locker.h"

#define DB_ATTACH_TIMEOUT       (30*1000)           //设置获取超时为30s


//数据库连接池类
class CDBConnPool;

//数据库连接
class CDBConn : public CDBCore
{
public:
	CDBConn(CDBConnPool *);
    CDBConn();
    ~CDBConn();

public:
    void Detach();
	void SetThreadID(int nThreadID){m_nThreadID = nThreadID;};
	int  GetThreadID()const {return m_nThreadID;};

private:
	CDBConnPool	*				m_pPool;            //绑定的连接池
	int							m_nThreadID;		//线程唯一ID 因为DBConn会被分配给多个线程 顺便把ID 
};


//数据库连接池类
class CDBConnPool
{
public:
	CDBConnPool();
	~CDBConnPool();

public:
	bool Open(int nMaxConn,const char *svr, const char *usr, const char *pwd, const char *db);
	void Close();

public:
	CDBConn *Lock();
	bool Unlock(CDBConn *);

protected:
	std::list<CDBConn *>        m_pFreeList;                //空队列
	CMyCriticalSection            m_csFreeList;               //空队列唯一使用

	char                        m_Server[64];               //服务器
	char						m_UserName[128];            //用户名
	char						m_Password[128];            //密码
	char						m_Database[64];             //数据库

	int							m_nConnCount;				//连接数
};

class CDBOperator
{
	CDBConn *m_pObject;

public:
	CDBOperator(CDBConnPool *);
	~CDBOperator();
	CDBConn* GetConn()		{ return m_pObject;}
	CDBConn* operator->()	{ return m_pObject;}
};

void ConvertHex(char *pBuff,const unsigned char *pData,int iSize);

#endif

