// AccountServer.cpp : 定义控制台应用程序的入口点。
//

#include <tchar.h>

#include "WorldServer.h"
//#include "Common/Script.h"

//for test
#include "CommLib/SimpleDataPool.h"
#include "common/playerStructEx.h"
#include "Script/lua_tinker.h"
#include "Script/LuaConsole.h"
#include "Common/CfgBuilder.h"

lua_State* L = 0;
extern "C" int luaopen_snapshot(lua_State *L);

int _tmain(int argc, _TCHAR* argv[])
{
    /*if (EventGuard::CheckExist("md_worldserver_"))
	{
		g_Log.WriteWarn("一台机器只能允许开1个world server");
		return 0;
	}

    EventGuard eventGuard("md_worldserver_");*/
	CCfgBuilder::ParseCfgBuilder(argc, argv);
	L = lua_open();
	luaL_openlibs(L);
	luaopen_snapshot(L);
	lua_tinker::init(L);


	SERVER_CLASS_NAME *pServerInstance = SERVER_CLASS_NAME::GetInstance();

	if(!pServerInstance->Initialize())
	{
		delete pServerInstance;
		return 0;
	}

	LuaConsole console;

	pServerInstance->StartService();

	pServerInstance->Maintenance();

	pServerInstance->DenyService();

	pServerInstance->StopService();

	lua_close(L);

	delete pServerInstance;
	return 0;
}

