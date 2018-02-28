#include "LogServer.h"
#include "Common/dbStruct.h"
#include "Common/CfgBuilder.h"

U32 g_runMode = 0;

int main(int argc, _TCHAR* argv[])
{
	HeapSetInformation(NULL,HeapEnableTerminationOnCorruption,NULL,0);
    ServerCloseMonitor monitor(g_runMode);
	CCfgBuilder::ParseCfgBuilder(argc, argv);
	CLogServer::getInstance()->init( "LogServer");
	CLogServer::getInstance()->main(g_runMode);
	CLogServer::getInstance()->shutdown();

	delete CLogServer::getInstance();
	return 0;
}		  