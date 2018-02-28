#ifndef __MYLOG__
#define __MYLOG__

#include <WinSock2.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include "base/Locker.h"

#define MAX_LOG_BUF 4096

class CWorkQueue;

typedef void (*screenCallback)(int type);

enum LogType
{
	LG_INPUTPROMPT,
	LG_INPUT,

	LG_NORMAL,
	LG_WARN,
	LG_ERROR,
	LG_FOCUS,
};

struct stFile
{
	HANDLE m_File;
	int m_Year,m_Mon,m_Day;
	char m_Filename[MAX_LOG_BUF];
	char m_Suffix[32];
	CMyCriticalSection m_cs;

	stFile()
	{
		m_Year=0;
		m_Mon=0;
		m_Day=0;
		m_File = INVALID_HANDLE_VALUE;
		m_Suffix[0] = 0;
		m_Filename[0] = 0;
	}


	~stFile()
	{
		if(m_File!=INVALID_HANDLE_VALUE)
			CloseHandle(m_File);
	}
};

struct LogItem
{
	LogType type;
    bool    logScreen;
    stFile  *fLog;
    stFile  *fErr;
	int bufSize;
	char *logBuf;
};

class CMyLog
{
	HANDLE m_Thread;
	HANDLE m_hCon;
	WORD m_colorNormal;
	WORD m_colorWarn;
	WORD m_colorError;

	stFile m_fLog;
	stFile m_fErr;

	std::string m_LogName;
	std::string m_Dir;
	bool m_LogFile;
	bool m_ErrFile;
	bool m_LogScreen;
	bool m_Zone;
	bool m_SyncMode;
	int  m_MaxIdelTime;
	unsigned int  m_lastTime;
	CMyCriticalSection m_cs;

	screenCallback m_pCallback;

	CWorkQueue *m_pWorkQueue;

protected:
	int CreateAFile(SYSTEMTIME &systime,stFile &file);
	int WriteAFile(HANDLE file,const char *pBuf,int BufSize);
	void postLogEvent(std::string &str,LogType type);
	bool testAvalibe(LogType type);
	void doWriteErr(LogItem *pItem);

public:
	CMyLog();
	~CMyLog();

	static int	EventProcess(LPVOID Param);
	void doWriteLog(LogItem *pItem);

	bool initConsole(bool flag = true);
	void SetLogName(const std::string& name,bool onScreen=true,bool inFile=true,bool inErr=true,bool sync=false);
	bool FindDir(std::string& dir);
	void SetSyncMode(bool sync);
	void SetIdleWarning(int maxIdleTime);
	void SetCallback(screenCallback pCallback);

	void showInput(std::string& str);
	void showPrompt(std::string& str);

	void Write(LogType type, const char* format, ... );
	void Write(LogType type, std::string& str);
	void Write(LogType type, std::stringstream &str);


	void WriteLog( const char* format, ... );
	void WriteLog(std::string& str)				{	Write(LG_NORMAL,str);	};
	void WriteLog(std::stringstream &str)		{	Write(LG_NORMAL,str);	};

	void WriteWarn( const char* format, ... );
	void WriteWarn(std::string& str)				{	Write(LG_WARN,str);		};
	void WriteWarn(std::stringstream &str)		{	Write(LG_WARN,str);		};

	void WriteError( const char* format, ... );
	void WriteError(std::string& str)			{	Write(LG_ERROR,str);	};
	void WriteError(std::stringstream &str)		{	Write(LG_ERROR,str);	};

	void WriteFocus( const char* format, ... );
	void WriteFocus(std::string& str)			{	Write(LG_FOCUS,str);	};
	void WriteFocus(std::stringstream &str)		{	Write(LG_FOCUS,str);	};
};

extern CMyLog g_Log;

#endif