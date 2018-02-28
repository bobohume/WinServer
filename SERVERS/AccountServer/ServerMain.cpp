// AccountServer.cpp : 定义控制台应用程序的入口点。
//
#include <winsock2.h>
#include <TChar.h>
#include "AccountServer.h"
#include "Common/Script.h"
#include "Common/build_no.h"
#include "Common/PacketType.h"
#include "Common/CfgBuilder.h"

int _tmain(int argc, _TCHAR* argv[])
{
	SERVER_CLASS_NAME *pServerInstance = SERVER_CLASS_NAME::GetInstance();
	CCfgBuilder::ParseCfgBuilder(argc, argv);
	if(!pServerInstance->Initialize())
	{
		delete pServerInstance;
		return 0;
	}

	pServerInstance->StartService();
	pServerInstance->Maintenance();
	pServerInstance->DenyService();
	pServerInstance->StopService();

	delete pServerInstance;
	return 0;
}

