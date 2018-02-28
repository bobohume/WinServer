#ifndef _SIMPLEDATAPOOL_H_
#define _SIMPLEDATAPOOL_H_

#include "base/Locker.h"
#include "CommLib.h"

//简单的池结构,如果分配不出来了,调用系统new来分配
template<typename _Ty>
class SimpleDataPool
{
	class CData : public _Ty
	{
	public:
		CData(void)
		{
			m_poolIdx = -1;	//-1不表示为空,-2表示不是自己分配的
		}

		void SetPoolId(int id)	   {m_poolIdx = id;}
		int  GetPoolId(void) const {return m_poolIdx;}

		int  m_poolIdx;
	};
public:
	SimpleDataPool(void)
	{
		m_isSystemAlloc = true;
		m_nPosition = 0;
		m_maxCount  = 0;
		m_pBuf  = 0;
		m_pObjs = 0;
	}

	bool Init(U32 maxCount,bool isSystemAlloc = true)
	{
		OLD_DO_LOCK(m_cs);

		m_isSystemAlloc = isSystemAlloc;
		m_maxCount	= maxCount;
		m_nPosition = 0;
		m_pBuf      = new CData[m_maxCount];
		m_pObjs     = new CData*[m_maxCount];
			
		if (0 == m_pBuf || 0 == m_pObjs)
		{
			assert(0 != m_pBuf);
			assert(0 != m_pObjs);
			return false;
		}

		for (U32 i = 0; i < m_maxCount; ++i)
		{
			m_pObjs[i] = &m_pBuf[i];
		}

		return true;
	}

	~SimpleDataPool(void)
	{
		OLD_DO_LOCK(m_cs);

		SAFE_DELETE_ARRAY(m_pBuf);
		SAFE_DELETE_ARRAY(m_pObjs);
		m_nPosition = 0;
	}

	_Ty* Alloc(void)
	{
		OLD_DO_LOCK(m_cs);

		if (0 != m_pObjs && 0 != m_pBuf)
		{
			if ( m_nPosition < m_maxCount )
			{
				CData *pObj = m_pObjs[m_nPosition];
				pObj->SetPoolId(m_nPosition);

				m_nPosition++;
				return pObj;	
			}	
		}
		
		if (!m_isSystemAlloc)
			return 0;

		CData* pData = new CData;
		pData->SetPoolId(-2);
		return pData;
	}

	void Free(_Ty* ptr)
	{
		assert( ptr != 0 );	
        
        if (ptr == 0)
        {
            return;
        }

		CData* pObj = (CData*)ptr;

		OLD_DO_LOCK(m_cs);

		if (-2 == pObj->GetPoolId())
		{
			delete pObj;
			return;
		}
        
		//this is from pool
        assert( m_nPosition > 0 );
        
        if ( m_nPosition <= 0 )
        {
            return;
        }
        
        U32 uDelIndex = pObj->GetPoolId();
        assert(uDelIndex < m_nPosition );
        
        if (uDelIndex >= m_nPosition )
        {
            return;
        }
        
        m_nPosition--;
        CData *pDelObj			= m_pObjs[uDelIndex];
        m_pObjs[uDelIndex]	    = m_pObjs[m_nPosition];
        m_pObjs[m_nPosition]	= pDelObj;

        m_pObjs[uDelIndex]->SetPoolId(uDelIndex );
        m_pObjs[m_nPosition]->SetPoolId(-1);
	}

	U32 GetTotalCount(void) 
	{
		OLD_DO_LOCK(m_cs);
		return m_nPosition;
	}
private:
	CMyCriticalSection m_cs;

	CData*  m_pBuf;
	CData** m_pObjs;
	U32     m_nPosition;
	U32		m_maxCount;

	bool	m_isSystemAlloc;
};

#endif /*_SIMPLEDATAPOOL_H_*/