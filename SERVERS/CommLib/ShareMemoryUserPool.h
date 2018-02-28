#ifndef _SHAREMEMORYUSERPOOL_H_
#define _SHAREMEMORYUSERPOOL_H_

#include "base/Locker.h"
#include <hash_map>
#include "ShareMemoryPool.h"

#define INVALID_POOL_IDX -1

//保存要被共享的逻辑数据信息,并提供相应的接口来保存或删除数据
//数据通过相应的pool创建
//数据由相应的uid标识,如果uid <= 0,则数据为无效数据
//_Ty必须提供
//	1:Clear 清空数据功能

template<typename _Ty,typename _UID>
class SMData
{
	template<typename _Ty,typename _UID,int _MaxCount>
	friend class SMDataPool;

	typedef CWrapSM<_Ty> SM_DATA;
public:
	//操作前需要绑定逻辑数据
	void Attach(_Ty* pLogicData) {m_pLogicData = pLogicData;}
	_Ty* Get(void) {return m_pLogicData;}
	const _Ty* Get(void) const {return m_pLogicData;}

	//从sm导入数据
	bool LoadFromSM(void)
	{
		if (0 == m_pLogicData || 0 == m_pSMData)
		{
			assert(0);
			return false;
		}

		m_pSMData->Lock();

		if (!IsUsed())
		{
			m_pSMData->UnLock();
			return false;
		}
        
        memcpy(m_pLogicData,m_pSMData->GetData(),sizeof(_Ty));
        m_pSMData->UnLock();

		return true;
	}

    //更新数据到sm,如果saveToDB为true,
	//则提示sm服务器保存到数据库
    void Save(bool saveToDB = true,bool isLastSave = false)
    {
        if (0 == m_pSMData || 0 == m_pLogicData)
            return;
		
		assert(INVALID_POOL_IDX != m_poolIdx);
        
        m_pSMData->Lock();

		if (saveToDB)
			m_pSMData->SetEvent(SM_COPYDATA);

        memcpy(m_pSMData->GetData(),m_pLogicData,sizeof(_Ty));

		if (saveToDB)
		{
			m_pSMData->SetVersion(m_pSMData->GetVersion() + 1);
			m_pSMData->SetEvent(isLastSave ? SM_QUITSAVE : SM_READYSAVE);
		}

        m_pSMData->UnLock();
    }

	//给内存块设置退出保存标志
	void ForceFastSave(void)
	{
		if(0 == m_pSMData)
			return;

		m_pSMData->SetVersion(m_pSMData->GetVersion() + 1);
		m_pSMData->SetEvent(SM_QUITSAVE);
	}
    
    //删除数据,sm服务器根据具体的内容进行删除操作.
    void Delete(void)
    {
        if (0 == m_pSMData)
            return;
		
		m_pSMData->Lock();
        m_pSMData->SetEvent(SM_READYDEL);            
		m_pSMData->UnLock();
    }
	
	//对共享内存做标记
	int GetMark(void)
	{
		if (0 == m_pSMData)
            return -1;
		
		return m_pSMData->GetMark();
	}

	void SetMark(int mark)
	{
		if (0 == m_pSMData)
            return;
		
		m_pSMData->SetMark(mark);
	}

	int  GetPoolIdx(void)        {return m_poolIdx;}
	int  GetPoolIdx(void) const  {return m_poolIdx;}

	void Lock(void)     { if (m_pSMData) m_pSMData->Lock(); }
    void UnLock(void)   { if (m_pSMData) m_pSMData->UnLock(); }

	//判断数据是否有效
    bool IsSaved(void)       {return 0 == m_pSMData ? false : (SM_SAVED     == m_pSMData->GetEvent());}
	bool IsSaving(void)      {return 0 == m_pSMData ? false : (SM_READYSAVE == m_pSMData->GetEvent() || SM_QUITSAVE == m_pSMData->GetEvent());}
    bool IsDeleting(void)    {return 0 == m_pSMData ? false : (SM_READYDEL  == m_pSMData->GetEvent());}
    bool IsCopying(void)     {return 0 == m_pSMData ? false : (SM_COPYDATA  == m_pSMData->GetEvent());}
	bool IsUsed(void)	     {return 0 == m_pSMData ? false : (SM_NORMAL	== m_pSMData->GetStatus());}
	
	void Reset(void)
    {
		m_uid = 0;
		
		if (m_pLogicData)
			m_pLogicData->Clear();

		if (0 != m_pSMData)
		{
			m_pSMData->Lock();
			m_pSMData->SetEvent(SM_NOEVENT);
			m_pSMData->SetStatus(SM_FREE);
			m_pSMData->SetVersion(0);
			m_pSMData->UnLock();
		}

    }
protected:
	SMData(void)
    {
        m_poolIdx	 = INVALID_POOL_IDX;
		m_pLogicData = 0;
        m_pSMData	 = 0;
        m_uid		 = 0;
		m_val	     = 0;
    }
    
    ~SMData(void)
    {    
        m_pSMData	 = 0;
		m_pLogicData = 0;
        m_poolIdx	 = INVALID_POOL_IDX;            
    }
    
	void SetPoolIdx(int idx)     {m_poolIdx = idx;}

	void SetId(_UID id)		     {m_uid = id;}
	void SetSM(SM_DATA* pSMData) {m_pSMData = pSMData;}

	const _UID GetId(void) const {return m_uid;}
    _UID GetId(void)             {return m_uid;}
   
	bool SetUsed(int mark)
	{
		if (0 == m_pSMData)
			return false;

		if(SM_FREE != m_pSMData->GetStatus())
		{
			assert(0);
			return false;
		}
		
		m_pSMData->SetMark(mark);
		m_pSMData->SetStatus(SM_NORMAL);
		return true;
	}

	

    _UID     m_uid;
	_Ty*	 m_pLogicData;
    SM_DATA* m_pSMData;
    int      m_poolIdx;
	long     m_val;
};

//管理逻辑数据和sm数据的连接,这个只在逻辑端使用
template<typename _Ty,typename _UID,int _MaxCount>
class SMDataPool
{
public:
    typedef SMData<_Ty,_UID>	 POOL_DATA;
	typedef CWrapSM<_Ty>		 SM_DATA;
    
	SMDataPool(void)
	{
		m_offset = 0;
	}

    ~SMDataPool(void)
	{
	}

	int GetCount(void) const {return _MaxCount;}
	
	bool Init(int key)
	{
		OLD_DO_LOCK(m_cs);

		m_offset = 0;

		if (!m_pool.InitClient(_MaxCount,key))
			return false;

		//读取已经分配的项
		for(int i = 0; i < _MaxCount; ++i)
		{
			SM_DATA* pSMData = m_pool.GetObj(i);

			assert(0 != pSMData);

			m_pNodes[i].SetSM(pSMData);
			m_pNodes[i].SetPoolIdx(i);
		}

		return true;
	}

	//获取pool里的所有对象,用来启动时重建逻辑层的管理
	POOL_DATA* Get(int pos)
	{
		if (pos < 0 || pos >= _MaxCount)
		{
			assert(0);
			return 0;
		}

		OLD_DO_LOCK(m_cs);
		return &m_pNodes[pos];
	}
		
	POOL_DATA* Alloc(_UID id,int mark)
	{
		OLD_DO_LOCK(m_cs);

		//如果还在cache列表里,则直接获取
		UID_POOL_MAP::iterator iter = m_cacheList.find(id);

		if (iter != m_cacheList.end())
		{	
			POOL_DATA* pData = &m_pNodes[iter->second];
			m_cacheList.erase(iter);
			return pData;
		}
		
		int count = 0;

		if (m_offset >= _MaxCount)
			m_offset = 0;

		while (m_offset < _MaxCount && count++ < _MaxCount)
		{
			POOL_DATA* pData = &m_pNodes[m_offset];
			
			pData->Lock();

			if(!pData->IsUsed())
			{
				m_offset++;
				pData->SetId(id);
				
				if (pData->SetUsed(mark))
				{
					pData->UnLock();
					return pData;
				}
			}

			pData->UnLock();

			m_offset++;
			
			if (m_offset >= _MaxCount)
				m_offset = 0;
		}

		return 0;
	}
	
	//释放数据,如果数据在保存或删除过程中,则先放到缓冲列表里
	void Free(POOL_DATA* pData)
	{
		OLD_DO_LOCK(m_cs);

		if (0 == pData)
		{
			assert(0);
			return;
		}
		
		if (INVALID_POOL_IDX == pData->GetPoolIdx())
		{
			assert(0);
			return;
		}

        pData->Attach(0);
		
		//如果还有未处理的事件,则先不释放
		if (pData->IsSaving() || pData->IsDeleting())
		{
			m_cacheList[pData->GetId()] = pData->GetPoolIdx();
		}
		else
		{
			m_offset = pData->GetPoolIdx();
			pData->Reset();
		}
	}

	bool IsSaving(U32 id) const
	{
		OLD_DO_LOCK(m_cs);
		UID_POOL_MAP::const_iterator iter = m_cacheList.find(id);
		return iter != m_cacheList.end();
	}

	bool LoadCache(U32 id,_Ty* pData)
	{
		assert(0 != pData);

		OLD_DO_LOCK(m_cs);

		UID_POOL_MAP::const_iterator iter = m_cacheList.find(id);

		if (iter == m_cacheList.end())
			return false;
			
		POOL_DATA* pSMData = &m_pNodes[iter->second];

		if (0 == pSMData)
			return false;

		pSMData->Lock();

		if (pSMData->IsSaving())
		{
			memcpy(pData,pSMData->m_pSMData->GetData(),sizeof(_Ty));
			pSMData->UnLock();
			return true;
		}
		
		pSMData->UnLock();
		return false;
	}

	void Update(void)
	{
		OLD_DO_LOCK(m_cs);

		__FreeCacheList();
	}

	//如果是SM客户端,有时在启动时需要确认SM数据被保存
	void WaitSaved(int mark)
	{
		//确保pool内的所有数据被保存
		POOL_DATA* pSMPlayerData = 0;

		for (int i = 0; i < _MaxCount; ++i)
		{
			pSMPlayerData = Get(i);

			if (0 == pSMPlayerData)
				continue;

			//this was controled by app
			if (!pSMPlayerData->IsUsed() || pSMPlayerData->GetMark() != mark)
			{
				continue;
			}

			//这里先解锁,防止关闭时锁没有释放
			pSMPlayerData->UnLock();
			
			if(pSMPlayerData->IsSaving() || pSMPlayerData->IsDeleting())
			{
				pSMPlayerData->Lock();
				pSMPlayerData->ForceFastSave();
				pSMPlayerData->UnLock();
			}

			//这里没必要加锁,其他客户端模块不会操作这些变量
			while(pSMPlayerData->IsSaving() || pSMPlayerData->IsDeleting())
			{
				g_Log.WriteLog("正在等待共享内存数据保存...");
				Sleep(1000);
			}
		
			//释放这块
			pSMPlayerData->Reset();
		}
	}
private:
	void __FreeCacheList(void)
	{
		for (UID_POOL_MAP::iterator iter = m_cacheList.begin(); iter != m_cacheList.end();)
		{
			if (iter->second == INVALID_POOL_IDX)
			{
				iter = m_cacheList.erase(iter);
				continue;
			}

			POOL_DATA& poolData = m_pNodes[iter->second];

			if (poolData.IsSaving() || poolData.IsDeleting())
			{
				++iter;
				continue;
			}
			else
			{
				iter = m_cacheList.erase(iter);

				//释放这个data
				m_offset = poolData.GetPoolIdx();
				poolData.Reset();
			}
		}
	}

	CMyCriticalSection m_cs;

	typedef stdext::hash_map<_UID,int> UID_POOL_MAP;
	UID_POOL_MAP m_cacheList;

    POOL_DATA  m_pNodes[_MaxCount];
    U32        m_offset;
	
	ShareMemoryPool<_Ty> m_pool;
};

#endif /*_SHAREMEMORYUSERPOOL_H_*/