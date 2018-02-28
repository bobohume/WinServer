#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H
#include "LogServer.h"
#include <deque>
#include <assert.h>
#include "Common/dbStruct.h"
#include "Base/Locker.h"

#define DISABLE_DB_STAT 1

/* =========================================================================*/
/*	日志写数据线程 LogWriteThread											*/ 
/*																			*/ 
/* =========================================================================*/
class LogWriteThread
{
public:
	LogWriteThread();
	~LogWriteThread();
	void start();
	void run();
	void exit();
	HANDLE getHandle() { return mThread;}
private:
	HANDLE	 mThread ;	// 线程句柄
};

struct stDBStruct
{
	dbStruct* data;		//日志数据
	int times;			//尝试次数
};

/* =========================================================================*/
/*	日志线程管理器 CLogManager												*/ 
/*																			*/ 
/* =========================================================================*/
class CLogManager
{
public:
	CLogManager();
	~CLogManager();
	static CLogManager* GetInstance();

	bool Initialize(const char*, const char*, const char*, const char*, int);
	void Startup();
	bool ShutDown();
	void AddLogData(stDBStruct*);
	stDBStruct* GetLogData();
	size_t GetLogListSize();
	inline U32 GetDataBaseDate() { return mCurrentDBDate;}
	bool IsThreadLoop() { return mIsThreadLoop;}
	CDBConn* GetDBConn();
	bool ChangeDataBase();
	void StopThread();
	void HandleChangeDataBase(U32& DBDate, CDBConn** pDBConn);
private:
	LogWriteThread*				mThreads;				//日志写数据线程组
	int							mMaxThreads;			//最大线程数
	volatile bool				mIsThreadLoop;			//是否启用线程循环

	DataBase*					mCurrentDataBase;		//当前数据库管理对象
	DataBase*					mFreeDataBase;			//需要释放的数据库管理对象

	CMyCriticalSection			mCrit;					//临界锁
	std::deque<stDBStruct*>		mLogList;				//日志数据队列

	char						mServerName[32];		//服务器名
	char						mDBNamePrefix[32];		//当前数据库前缀名
	char						mUserName[32];			//用户名
	char						mPassword[32];			//密码
	U32							mMaxConns;				//最大数据库连接数	

	U32							mCurrentDBDate;			//当天日期
	U32							mThreadChangeDBCount;	//线程确定更换数据库对象次数
}; 

#define LOGMANAGER CLogManager::GetInstance()

#endif