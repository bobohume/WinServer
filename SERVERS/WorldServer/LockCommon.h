#ifndef LOCK_COMMON_H
#define LOCK_COMMON_H

#include <bitset>
#include "base/types.h"
#include "CommLib/SimpleLocker.h"

//兼容之前的全局锁
extern SimpleLock g_lock;

namespace Lock
{
    //暂时放着这些,用来兼容之前的代码
    enum LockStatus
    {
        Player		= BIT(0),
        Server		= BIT(1),
        Team		= BIT(2),
        Chat		= BIT(4),
        Copymap		= BIT(5),
        Mail		= BIT(6),
        Social		= BIT(7),
        Channel		= BIT(8),
        Top			= BIT(9),
        Designation = BIT(10),
        Board		= BIT(11),
        Stock		= BIT(12),
        Achievement	= BIT(13),
        Org			= BIT(14),
        Sync		= BIT(15),
        LogicEvent	= BIT(16),
        ZoneEvent	= BIT(17),
        Billing		= BIT(18),
        League		= BIT(19),
        Stall		= BIT(20),
        Bonus       = BIT(21),
        GoldExchage	= BIT(22),
    };
}

#ifndef DEFAULT_DO_LOCK

#ifdef DO_LOCK
	#undef DO_LOCK
#endif

//这里换成精简的lock方式
#define DO_LOCK(p) AutoLock __lock##__COUNTER__(g_lock); 
#endif

#endif
