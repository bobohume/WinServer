//#include "stdafx.h"
#include "DB_CurlRecovery.h"
#include "DBContext.h"
#include "CommLib\DBThreadManager.h"
#include "Common\MemGuard.h"
#include "..\RemoteAccount.h"
#include "Common\Http\Request.h"

DB_CurlRecovey::DB_CurlRecovey():mCurl(NULL)
{
}

int DB_CurlRecovey::Execute(int ctxId, void* param)
{
	if (DB_CONTEXT_NOMRAL == ctxId)
	{
		do 
		{
			if(mCurl == NULL)
				break;

			curl_easy_cleanup(mCurl);
		} while (false);
	}
	return PACKET_OK;
}