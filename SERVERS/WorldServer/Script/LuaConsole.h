#pragma once

#include <WinSock2.h>
#include <windows.h>

#include <string>
#include <hash_map>

class LuaConsole
{
public:
	LuaConsole();
	virtual ~LuaConsole();

	static LuaConsole* Instance();
    void printf( const char* format, ... );
	void gotoMark();
	void SetMarkPos();
private:
	static void workMethod( void* param );

    void _printf( const char* format, ... );
	void executeCmd(const char* cmd, int size);

	void process();
	static const int MaxCmdCount = 20;

	HANDLE mStdInput;
	HANDLE mStdOutput;
	HANDLE mStdError;

	char inbuf[512];
	char rgCmds[MaxCmdCount][512];
	int  iCmdIndex;
	int  inpos;
	COORD mMarkPos;

	std::string mPromote;
	HANDLE mWorkThread;

    bool m_isEnded;
	int	  mLastLineCount;
};


