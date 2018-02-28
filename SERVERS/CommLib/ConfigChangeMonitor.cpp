#include "ConfigChangeMonitor.h"
#include "CommLib.h"
#include "Thread.h"
#include <sys/stat.h>



ConfigChangeMonitor::ConfigChangeMonitor(void)
{
	m_pWorkQueue = 0;
	m_isEnded    = false;
	m_pThread    = 0;
}

ConfigChangeMonitor::~ConfigChangeMonitor()
{
	if (0 != m_pThread)
	{
		m_isEnded = true;
		m_pThread->WaitForCompletion();
	}

	SAFE_DELETE(m_pThread);
}

int ConfigChangeMonitor::run(void)
{
	DWORD dwWaitStatus = 0;

    if (0 == m_handles.size() || 0 == m_paths.size())
    {
        return 0;
    }

	while (!m_isEnded) 
	{
		{
			CLocker lock(m_Cs);
			int numHandles = m_handles.size();

			//Wait for notification.
			dwWaitStatus = WaitForMultipleObjects(numHandles, &m_handles[0], FALSE, 3000);

			if (dwWaitStatus >= WAIT_OBJECT_0 && dwWaitStatus < WAIT_OBJECT_0 + numHandles)
			{
				int iPath = dwWaitStatus - WAIT_OBJECT_0;

				FindNextChangeNotification(m_handles[iPath]);

				bool isDisabled = false;

				//查找修改的文件
				for (size_t i = 0; i < m_files.size(); ++i)
				{
					FileItem& item = m_files[i];

					if (item.pathidx != iPath)
						continue;

					item.checkModify = true;
					item.changeCount++;

					if (item.changeCount > 10000)
					{
						//修改太频繁了，屏蔽掉
						isDisabled = true;
					}
				}

				if (isDisabled)
				{
					FindCloseChangeNotification(m_handles[iPath]);
					m_handles[iPath] = 0;
				}
			}
			else
			{
				//发送文件变更事件
				for (size_t i = 0; i < m_files.size(); ++i)
				{
					FileItem& item = m_files[i];

					if (!item.checkModify)
						continue;

					struct _stat fInfo;
					int rslt = _stat(item.fileName.c_str(), &fInfo);

					if (0 == rslt)
					{
						if (item.ftLastWriteTime != fInfo.st_mtime && 0 != fInfo.st_size)
						{
							item.ftLastWriteTime = fInfo.st_mtime;

							g_Log.WriteLog("文件[%s]检测到被修改", item.fileName.c_str());
							m_pWorkQueue->PostEvent(i, (void*)item.fileName.c_str(), strlen(item.fileName.c_str()) + 1, true, WQ_CONFIGMONITOR);
						}
					}

					item.checkModify = false;
				}
			}
		}
            Sleep(1000);
	}

	for (size_t i = 0; i < m_handles.size(); ++i)
	{
        if (0 == m_handles[i])
            continue;

		FindCloseChangeNotification(m_handles[i]);
	}
	
	return 0;
}

void ConfigChangeMonitor::addMonitor(std::string file)
{
	struct _stat fInfo;
	CLocker lock(m_Cs);
	{
		int result = _stat(file.c_str(), &fInfo);

		if (0 == result)
		{
			FileItem item;

			item.fileName = file;
			item.ftLastWriteTime = fInfo.st_mtime;
			item.pathidx = -1;
			item.changeCount = 0;
			item.checkModify = false;

			m_files.push_back(item);
		}
	}

	//获取监控的目录
	{
		FileItem& item = m_files.back();

		char dir[256] = { 0 };
		_splitpath_s(item.fileName.c_str(), 0, 0, dir, 256, 0, 0, 0, 0);

		if (0 == strlen(dir))
			return;

		size_t iPath = 0;

		for (; iPath < m_paths.size(); ++iPath)
		{
			if (0 == _stricmp(dir, m_paths[iPath].c_str()))
				break;
		}

		if (iPath >= m_paths.size())
		{
			HANDLE dwhandle = FindFirstChangeNotification(dir, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);

			//插入新的路径
			if (dwhandle != INVALID_HANDLE_VALUE)
			{
				item.pathidx = m_paths.size();

				m_paths.push_back(dir);
				m_handles.push_back(dwhandle);
			}
			else
			{
				g_Log.WriteWarn("\n文件%s监控失败", item.fileName.c_str());
			}
		}
		else
		{
			item.pathidx = iPath;
		}
	}
}

void ConfigChangeMonitor::setMonitor(const std::vector<std::string>& files)
{
    struct _stat fInfo;
    
    for (size_t i = 0; i < files.size(); ++i)
    {
        int result = _stat(files[i].c_str(),&fInfo);
        
        if (0 == result)
        {
            FileItem item;

            item.fileName        = files[i];
            item.ftLastWriteTime = fInfo.st_mtime;
            item.pathidx         = -1;
            item.changeCount     = 0;
            item.checkModify     = false;

            m_files.push_back(item);
        }
    }

    //获取监控的目录
    for (size_t i = 0; i < m_files.size(); ++i)
    {
        FileItem& item = m_files[i];
        
        char dir[256] = {0};
		_splitpath_s(item.fileName.c_str(),0,0,dir,256,0,0,0,0);

        if (0 == strlen(dir))
            continue;

        size_t iPath = 0;

        for (;iPath < m_paths.size(); ++iPath)
        {
            if (0 == _stricmp(dir,m_paths[iPath].c_str()))
                break;
        }
        
        if (iPath >= m_paths.size())
        {
            HANDLE dwhandle = FindFirstChangeNotification(dir,FALSE,FILE_NOTIFY_CHANGE_LAST_WRITE);

            //插入新的路径
            if (dwhandle != INVALID_HANDLE_VALUE)
            {
                item.pathidx = m_paths.size();

                m_paths.push_back(dir);
                m_handles.push_back(dwhandle);
            }
            else
            {
                g_Log.WriteWarn("\n文件%s监控失败",item.fileName.c_str());
            }
        }
        else
        {
            item.pathidx = iPath;
        }
    }
}

void ConfigChangeMonitor::Start(CWorkQueue* workQueue,const std::vector<std::string>& items)
{
    if (0 != m_pThread)
        return;

    if (0 == workQueue || 0 == items.size())
        return;
    
    m_pWorkQueue = workQueue;
    m_pThread    = CThread::Create(this);
    
    setMonitor(items);
	m_pThread->Resume();
}