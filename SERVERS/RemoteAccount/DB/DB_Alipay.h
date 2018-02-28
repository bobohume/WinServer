#ifndef _DB_ALIPAY_H_
#define _DB_ALIPAY_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"

class DB_Alipay 
	:public ThreadBase
{
public:
	DB_Alipay(const AlipayInfo& info);

	virtual int Execute(int ctxId, void* param);

	AlipayInfo info;
};

#endif /*_DB_ALIPAY_H_*/