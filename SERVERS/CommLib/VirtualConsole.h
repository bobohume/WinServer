#pragma once

#include <WinSock2.h>
#include <windows.h>

#include <string>
#include <hash_map>

class VirtualConsole
{
public:
	VirtualConsole();
	virtual ~VirtualConsole();

    void printf( const char* format, ... );
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

	std::string mPromote;
	HANDLE mWorkThread;

    bool m_isEnded;
};

typedef void ( *CmdFunction )(void*);
typedef stdext::hash_map< std::string,CmdFunction> HashCmdFunctionMap;

struct CmdFunctionBuilder
{
	CmdFunctionBuilder(const char* name, CmdFunction funct);
	static HashCmdFunctionMap& getInst();
};

#define CmdFunction(name) \
	static void cmd##name(void* args); \
	static CmdFunctionBuilder name##builder( #name, cmd##name ); \
	static void cmd##name(void* args)



