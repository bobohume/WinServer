#ifndef _DATACACHE_H_
#define _DATACACHE_H_

//数据缓冲池
//缓冲区放满后,会先删除最先放入的数据
#include "CommLib.h"
#include <hash_map>
#include <time.h>
#include <string>
#include "SimpleDataPool.h"
#include "base/log.h"

template<typename _Ty,int _MaxCount,int _Expire>
class DataCache
{
	struct Data
	{
	public:
		Data(void)
		{
			m_lastAccessTime = (U32)_time32(0);
			m_poolId = 0;
		}

		void Update(void)
		{
			m_lastAccessTime = (U32)_time32(0);
		}	

		void SetPoolId(int poolID) { m_poolId = poolID;}
		int	 GetPoolId(void) const { return m_poolId;}

		U32 m_lastAccessTime;
		U32 m_poolId;
		_Ty m_data;
	};
public:
	DataCache(void)
	{
		bool rslt = m_pools.Init(_MaxCount,false);
		m_clearFlag = false;
		assert(rslt);
	}

	~DataCache(void)
	{
		OLD_DO_LOCK(m_cs);
		m_dataMaps.clear();
	}

	bool Add(U32 id,const char* name,_Ty& data)
	{
		OLD_DO_LOCK(m_cs);

		ID_DATA_MAP::iterator iter = m_dataMaps.find(id);

		if (iter != m_dataMaps.end())
		{
			Data* refData = iter->second;
			refData->m_data = data;
			refData->Update();

			return true;
		}

		if (m_dataMaps.size() >= _MaxCount)
		{	
			U32 currentTime = (U32)_time32(0);
			
			U32 freeCount = 0;

			g_Log.WriteWarn("缓冲区已满...当前缓冲区大小:[%d]...开始清理缓存...", m_dataMaps.size());
			//缓冲区已满,释放很久没访问的数据
			if (m_clearFlag)
			{
				for (ID_DATA_MAP::iterator iter = m_dataMaps.begin(); iter != m_dataMaps.end();)
				{
					Data* refData = iter->second;

					if (currentTime > refData->m_lastAccessTime + _Expire)
					{	
						refData->m_data.reset();
						m_pools.Free(refData);
						iter = m_dataMaps.erase(iter);

						freeCount++;

						if (freeCount > 0.2 * _MaxCount)
							break;
					}
					else
					{
						++iter;
					}
				}
				m_clearFlag = false;
			}
			else
			{
				for (ID_DATA_MAP::reverse_iterator iter = m_dataMaps.rbegin(); iter != m_dataMaps.rend();)
				{
					Data* refData = iter->second;

					if (currentTime > refData->m_lastAccessTime + _Expire)
					{	
						refData->m_data.reset();
						m_pools.Free(refData);
						iter = ID_DATA_MAP::reverse_iterator(m_dataMaps.erase(--iter.base()));

						freeCount++;

						if (freeCount > 0.2 * _MaxCount)
							break;
					}
					else
					{
						++iter;
					}
				}
				m_clearFlag = true;
			}
			g_Log.WriteWarn("缓存清理完毕...当前缓冲区大小:[%d]...清理数量:[%d]...", m_dataMaps.size(), freeCount);
		}

		Data* pData = m_pools.Alloc();

		if (0 == pData)
		{
			return false;
		}

		pData->Update();

		pData->m_data = data;
		m_dataMaps[id] = pData;

		if (0 != name)
			m_nameMaps[name] = id;

		return true;
	}

	void Del(U32 id)
	{
		OLD_DO_LOCK(m_cs);

		ID_DATA_MAP::iterator iter = m_dataMaps.find(id);

		if (iter == m_dataMaps.end())
		{
			return;
		}
		
		Data* refData   = iter->second;
		refData->m_data.reset();
		m_pools.Free(refData);
		m_dataMaps.erase(iter);
	}

	bool Load(U32 id,_Ty& pOutput)
	{
		OLD_DO_LOCK(m_cs);

		ID_DATA_MAP::iterator iter = m_dataMaps.find(id);

		if (iter == m_dataMaps.end())
			return false;

		Data* pData = iter->second;
		
		if (0 == pData)
		{
			assert(0);
			return false;
		}
		
		pOutput = pData->m_data;
		pData->Update();
		
		return true;
	}

	bool Load(const char* name,_Ty& pOutput)
	{
		OLD_DO_LOCK(m_cs);

		NAME_DATA_MAP::iterator iter = m_nameMaps.find(name);

		if (iter == m_nameMaps.end())
			return false;

		return Load(iter->second,pOutput);
	}

	bool IsCached(U32 id)
	{
		OLD_DO_LOCK(m_cs);
		return m_dataMaps.find(id) != m_dataMaps.end();
	}
protected:
	CMyCriticalSection m_cs;
	
	bool m_clearFlag;

	typedef SimpleDataPool<Data> DATA_POOL;
	DATA_POOL m_pools;

	typedef stdext::hash_map<U32,Data*> ID_DATA_MAP;
	ID_DATA_MAP m_dataMaps;

	typedef stdext::hash_map<std::string,U32> NAME_DATA_MAP;
	NAME_DATA_MAP m_nameMaps;
};

#endif /*_DATACACHE_H_*/