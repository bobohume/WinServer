#ifndef _DB_CURL_RECOVERY_H_
#define _DB_CURL_RECOVERY_H_

#include "CommLib/ThreadPool.h"
#include "Common/Common.h"
#include "Common/Http/curl/curl.h"

class DB_CurlRecovey
	: public ThreadBase
{
public:
	DB_CurlRecovey();

	virtual int Execute(int ctxId, void* param);

	CURL* mCurl;
};

#endif /*_DB_CURL_RECOVERY_H_*/