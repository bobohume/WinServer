#ifndef _COPYMAPSMDATA_H_
#define _COPYMAPSMDATA_H_

#include "CommLib/ShareMemoryKey.h"
#include "CommLib/ShareMemoryUserPool.h"

class CopymapSMData
{
public:
	int	 nId;
	int	 m_dataId;
	int	 nMapId;
	int	 nLineId;
    int  m_startTime;
    int  m_closeTime;
    int  m_prograss;
    U32  m_arenaId;
    U32  m_combatId;
	bool m_isClosed;

	CopymapSMData()
	{
		Clear();
	}

    void Clear(void)
    {
        nId			 = 0;
		m_dataId	 = 0;
		nMapId		 = 0;
		nLineId		 = 0;
        m_startTime  = 0;
        m_closeTime  = 0;
        m_arenaId    = 0;
        m_combatId   = 0;
		m_isClosed	 = false;
        m_prograss   = 0;
    }
};

#define SM_MAXCOPYMAPCOUNT 100000
typedef SMDataPool<CopymapSMData,U32,SM_MAXCOPYMAPCOUNT> SM_COPYMAP_POOL;
typedef SMDataPool<CopymapSMData,U32,SM_MAXCOPYMAPCOUNT>::POOL_DATA SM_COPYMAP_DATA;

#endif /*_COPYMAPSMDATA_H_*/