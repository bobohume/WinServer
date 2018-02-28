#ifndef TBL_GLOBALEVENT_H
#define TBL_GLOBALEVENT_H

#ifndef __DATABASE_H__
#include "DataBase.h"
#endif

#ifndef __TBLBASE_H__
#include "TBLBase.h"
#endif

#include "Common/GlobalEventBase.h"

class TBLGlobalEvent : public TBLBase
{
public:
	TBLGlobalEvent(DataBase* db);
	DBError Load(GlobalEvents& events);
	DBError Update(const GlobalEvent& event);
} ;

#endif
