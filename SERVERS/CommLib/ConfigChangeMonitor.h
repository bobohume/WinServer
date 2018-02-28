#ifndef _CONFIGCHANGEMONITOR_H_
#define _CONFIGCHANGEMONITOR_H_

#include "CommLib.h"
#include "ThreadProcedure.h"
#include "StdAfx.h"
#include "base/WorkQueue.h"
#include "SimpleTimer.h"
#include <string>
#include <vector>

class CThread;

//开启1个线程用来监控配置文件改写事件
class ConfigChangeMonitor : public ThreadProcedure
{
    struct FileItem
    {
        int         pathidx;
        int         changeCount;
        std::string fileName;
        __time64_t  ftLastWriteTime;
        bool        checkModify;
    };
public:
	ConfigChangeMonitor(void);
	~ConfigChangeMonitor();

	virtual int run(void);

	typedef void* HANDLE;
    
    //workQueue一定要是主逻辑的线程
	void Start(CWorkQueue* workQueue,const std::vector<std::string>& files);
	void addMonitor(std::string file);

private:
    void setMonitor(const std::vector<std::string>& items);

	bool m_isEnded;
    
	std::vector<HANDLE>      m_handles;
	std::vector<std::string> m_paths;
    std::vector<FileItem>    m_files;

	CThread*    m_pThread;
	CWorkQueue* m_pWorkQueue;
	CMyCriticalSection m_Cs;
};

#endif /*_SERVERCLOSEMONITOR_H_*/