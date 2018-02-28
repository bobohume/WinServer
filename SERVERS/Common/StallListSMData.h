#ifndef __STALLLISTSMDATA_H__
#define __STALLLISTSMDATA_H__

#include "CommLib/SimpleFlag.h"
#include "DBLayer\Data\TBLStallList.h"
#include "CommLib/ShareMemoryKey.h"
#include "CommLib/ShareMemoryUserPool.h"
#include <time.h>
#include "Common/PlayerStruct.h"
#include "common/FixedStallBase.h"

class StallSMData
{
public:
    enum STALL_MASK
    {
        MASK_INFO           = BIT(0),
        MASK_SELLITEM       = BIT(1),
        MASK_SELLITEM_ALL   = BIT(2),
        MASK_BUYITEM        = BIT(3),
        MASK_BUYITEM_ALL    = BIT(4),
        MASK_SELLPET        = BIT(5),
        MASK_SELLPET_ALL    = BIT(6),
		MASK_ALL			= BIT(0) | BIT(2) | BIT(4) | BIT(6),
    };

	StallSMData(void) {	Clear();}

	void Clear(void) { memset(this, 0, sizeof(StallSMData));}

    //数据标记,用来确定哪个数据需要被更新
    U32  m_dirtyMask;
    bool m_maskSellItem[MAX_FIXEDSTALL_SELLITEM];
    bool m_maskBuyItem[MAX_FIXEDSTALL_BUYITEM];
    bool m_maskSellPet[MAX_FIXEDSTALL_SELLPET];
    
    //清除所有标记数据
    void ClearSaveMask(void)
    {
        m_dirtyMask = 0;
        memset(m_maskSellItem,0,sizeof(m_maskSellItem));
        memset(m_maskBuyItem,0,sizeof(m_maskBuyItem));
        memset(m_maskSellPet,0,sizeof(m_maskSellPet));
    }

	stFixedStall		m_Info;
	stFixedStallItem	m_SellItem[MAX_FIXEDSTALL_SELLITEM];
	stFixedStallItem	m_BuyItem[MAX_FIXEDSTALL_BUYITEM];
	stFixedStallPet		m_SellPet[MAX_FIXEDSTALL_SELLPET];
};

#define SM_MAXSTALLLIST 250
typedef SMDataPool<StallSMData,U32,SM_MAXSTALLLIST> SM_STALL_POOL;
typedef SMDataPool<StallSMData,U32,SM_MAXSTALLLIST>::POOL_DATA SM_STALL_DATA;

#endif//__STALLLISTSMDATA_H__