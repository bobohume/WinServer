#ifndef WAIT_OBJECT_H
#define WAIT_OBJECT_H

#include <signal.h>
#include <string>

#include <Windows.h>

class CWaitObject
{
public:
	CWaitObject( std::string strName );
	virtual ~CWaitObject();
	static HANDLE m_hEvent;
	bool IsReady();
	int Wait( int time );
private:
	static void OnBreak( int );
	bool m_bReady;
};

#endif

