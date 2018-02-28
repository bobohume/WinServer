#include "WaitObject.h"

HANDLE CWaitObject::m_hEvent = NULL;

CWaitObject::CWaitObject( std::string strName )
{
	m_bReady = false;
	m_hEvent = OpenEventA( EVENT_ALL_ACCESS, FALSE, strName.c_str() );
	if( m_hEvent )
    {
        m_bReady = true;
		return ;
    }

	m_hEvent = CreateEventA( NULL, FALSE, FALSE, strName.c_str() );
	signal( SIGBREAK, OnBreak );
	signal( SIGINT, OnBreak );

	m_bReady = true;
}

CWaitObject::~CWaitObject()
{
	if( m_hEvent )
		CloseHandle( m_hEvent );
}

bool CWaitObject::IsReady()
{
	return m_bReady;
}

void CWaitObject::OnBreak(int)
{
	if( m_hEvent )
		SetEvent( m_hEvent );
}

int CWaitObject::Wait( int time )
{
	if( !m_bReady )
		return -1;

	if( WaitForSingleObject( m_hEvent, time ) == WAIT_OBJECT_0 )
		return 1;

	return 0;
}