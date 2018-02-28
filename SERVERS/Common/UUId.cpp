#include "UUId.h"
#include <Rpc.h>
#include <assert.h>
#include <time.h>
#include "BASE/types.h"

const char* uuid()
{
	UUID id;

	if (RPC_S_OK != UuidCreate(&id))
	{
		assert(0);
		return "";
	}

	static char sBuf[256] = {0};

	RPC_CSTR pstr = 0;
	UuidToString(&id,&pstr);
	strcpy(sBuf,(const char *)pstr);
	RpcStringFree(&pstr);

	return sBuf;
}