#include "Event.h"

void CEventSource::RegisterEventListener(CEventListener *pEventListener)
{
	m_listenerSet.insert( pEventListener );
}

void CEventSource::UnregisterEventListener(CEventListener *pEventListener)
{
	stdext::hash_set<CEventListener*>::iterator it;

	it = m_listenerSet.find( pEventListener );
	if( it != m_listenerSet.end() )
		m_listenerSet.erase( it );
}

void CEventSource::RaiseEvent( const stEventArg& eventArg )
{
	stdext::hash_set<CEventListener*>::iterator it;

	for( it = m_listenerSet.begin(); it != m_listenerSet.end(); it++ )
	{
		(*it)->OnEvent( this, eventArg );
	}
}

void CEventListener::ListenEvent( CEventSource* pEventSource )
{
	pEventSource->RegisterEventListener( this );
}