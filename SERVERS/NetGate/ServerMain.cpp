// AccountServer.cpp : 定义控制台应用程序的入口点。
//
#include <TChar.h>
#include "NetGate.h"
#include "Common/Script.h"
#include "Common/CfgBuilder.h"

const char* g_genGlobalName(const char* preStr)
{
	char path[256] = {0};
	GetModuleFileNameA(0, path, sizeof(path));
	static std::string name = preStr;
	name += "_MD_";
	char buf[256] = {0};
	_splitpath_s(path, 0, 0, 0, 0, buf,256, 0,0);
	name += buf;
	return name.c_str();
}

const int MAX_SAMEIPCONFIG = 32;
int _tmain(int argc, _TCHAR* argv[])
{
    /*if (EventGuard::CheckExist("md_netgate_"))
	{
		g_Log.WriteWarn("一台机器只能允许开1个net gate");
		return 0;
	}

    EventGuard eventGuard("md_netgate_");*/
	//HANDLE fileMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(bool)*MAX_SAMEIPCONFIG, g_genGlobalName("filemap"));
	//if (fileMap != NULL)
	{
		//DWORD error = GetLastError();
		/*bool* pView = (bool*)MapViewOfFile(fileMap, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
		if (pView != NULL)
		{
			HANDLE globalEvent = CreateEventA(NULL, FALSE, TRUE, g_genGlobalName("globalevent"));
			if (globalEvent != NULL)
			{
				WaitForSingleObject(globalEvent, INFINITE);
				if (error != ERROR_ALREADY_EXISTS)
					memset(pView, 0, sizeof(bool)*MAX_SAMEIPCONFIG);
				int i = 0;
				for (; i<argc; ++i)
				{
					if (_stricmp(argv[i], "-config")==0 && i+1<argc)
					{
						int config = atoi(argv[i+1]);
						if (config < MAX_SAMEIPCONFIG && !pView[config])
						{
							CFG_BUIILDER->SelectConfig(config);
							pView[config] = true;
							break;
						}
					}
				}
				if (i == argc)
					CFG_BUIILDER->SelectConfig(pView, MAX_SAMEIPCONFIG);
				SetEvent(globalEvent);*/
				CCfgBuilder::ParseCfgBuilder(argc, argv);
				SERVER_CLASS_NAME *pServerInstance = SERVER_CLASS_NAME::GetInstance();
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

				//WaitForSingleObject(globalEvent, INFINITE);
				//CFG_BUIILDER->ReleaseConfig(pView, MAX_SAMEIPCONFIG);
				//SetEvent(globalEvent);
				//CloseHandle(globalEvent);
			//}
			//UnmapViewOfFile(pView);
		//}
		//CloseHandle(fileMap);
	}

	return 0;
}

