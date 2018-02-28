// RANetLibClientTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ranetlib.h"
#include <iostream>
#include <windows.h>

char* strError[] = 
{
    "操作不存在，无法查询",
    "正在等待处理结果返回",
    "踢线操作超时",
    "已踢线",
    "帐号不在游戏中",
    "未知错误",
};

int _tmain(int argc, _TCHAR* argv[])
{
    RA_Connect("192.168.0.65",45100);

    while(true)
    {
        char name1[256] = {0};
        std::cin >> name1;

        int kick1 = RA_KickAccount(0,name1);
        
        while (true)
        {
            int rslt = RA_GetKickRespond(kick1);

            if (E_KICK_NORESP == rslt)
            {
                std::cout << "操作发起错误或结果已经被清除" << std::endl;
                break;
            }

            if (E_KICK_WAITTING == rslt)
            {
                std::cout << "正在等待结果返回..." << std::endl;
                Sleep(1000);
            }
            else
            {
                std::cout << "收到踢线返回结果:" << strError[rslt] << std::endl;
                break;
            }
        }
    }


	return 0;
}

