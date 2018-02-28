#pragma once

template< int _stateCount, typename _Ty >
class CStateMachine
{
public:
	typedef void ( _Ty::*StateTimeout )();
	typedef void ( _Ty::*StateUpdate )();
	typedef void ( _Ty::*StateTrigger )();

	CStateMachine()
		: currentState( 0 )
	{
		memset( mStateTimeOut, 0, sizeof( mStateTimeOut ) );
		memset( mStateUpdate, 0, sizeof( mStateUpdate ) );
		memset( mStateTrigger, 0, sizeof( mStateTrigger ) );

		memset( mTimeout, 0, sizeof( mTimeout ) );
		memset( mNextState, 0, sizeof( mNextState ) );
	}

	virtual ~CStateMachine()
	{

	}

	void registerState( int state, StateUpdate stateUpdate = 0, StateTrigger stateTriger = 0, DWORD timeout = 0, StateTimeout stateTimeOut = 0, int nextState = 0 )
	{
		mStateUpdate[state] = stateUpdate;
		mStateTimeOut[state] = stateTimeOut;
		mStateTrigger[state] = stateTriger;
		mTimeout[state] = ::GetTickCount() + timeout;
		mNextState[state] = nextState;
	}

	void update()
	{
		DWORD dwTick = ::GetTickCount();

		if( dwTick >= mTimeout[currentState] && mTimeout[currentState] != 0 )
		{
			if( mStateTimeOut[currentState] )
				(((_Ty*)this)->*mStateTimeOut[currentState])();

			if( mNextState[currentState] )
				setCurrentState( mNextState[currentState] );
		}
		else
		{
			if( mStateUpdate[currentState] )
				(((_Ty*)this)->*mStateUpdate[currentState])();
		}
	}

	void setCurrentState( int state )
	{
		if( state >= _stateCount )
			return ;

		if( currentState != state )
		{
			if( mStateTrigger[state] )
				((_Ty*)this->*mStateTrigger[state])();
			currentState = state;
		}
	}

	int getCurrentState()
	{
		return currentState;
	}

private:
	StateTimeout	mStateTimeOut[_stateCount];
	StateUpdate		mStateUpdate[_stateCount];
	StateTrigger	mStateTrigger[_stateCount];

	DWORD mTimeout[_stateCount];
	int mNextState[_stateCount];

	int currentState;
};