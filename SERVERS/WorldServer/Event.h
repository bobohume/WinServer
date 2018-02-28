#ifndef EVENT_H
#define EVENT_H

#include <hash_set>

struct stEventArg
{
	int nType;
	void* pParam;

	stEventArg()
	{
		nType = 0;
		pParam = NULL;
	}
} ;

class CEventSource;

class CEventListener
{
	friend class CEventSource;
protected:
	void ListenEvent( CEventSource* pEventSource );
	virtual void OnEvent( CEventSource* pEventSouce, const stEventArg& eventArg ) = 0;
} ;

class CEventSource
{
public:
	void RegisterEventListener( CEventListener* pEventListener );
	void UnregisterEventListener( CEventListener* pEventListener );
	void RaiseEvent( const stEventArg& eventArg );

private:
	stdext::hash_set<CEventListener*> m_listenerSet;
};

#endif