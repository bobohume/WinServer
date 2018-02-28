#ifndef _TIMERSTATEMACHINE_H_
#define _TIMERSTATEMACHINE_H_

#include <assert.h>
#include <time.h>

//去掉在在cot时this赋值的warning
#pragma warning(disable:4355)

/************************************************************************/
/* 简单状态机实现,每个状态都可以设置一个超时时间，当时间到达时调用相应的onExpire函数
/************************************************************************/
template<typename _parent,int _maxState,typename _eventHandler>
class CTimerStateMachine
{
public:
    typedef void (_parent::*default_handler)(void);

	struct State
    {
        State(void) : onEnter(0),onLeave(0),onExpire(0),onExec(0),onTrigle(0) {}

        default_handler  onEnter;
        default_handler  onLeave;
        default_handler  onExpire;
		default_handler  onExec;
        _eventHandler    onTrigle;
    };

    CTimerStateMachine(_parent* parent) : m_parent(parent),m_curState(-1),m_preState(-1),m_expireTime(0),m_onStateChanged(0) {}
    
    State& operator[](int index) {return m_states[index];}
    
    /************************************************************************/
    /* 切换状态，expireTime = 0表示不存在超时,isLoop表示是否循环设置超时
    /************************************************************************/
    void SetState(int state,int expireTime = 0,bool isLoop = false)
    {
        assert(state >= 0 && state < _maxState && "invalid state");

        if (state >= _maxState)
            return;

        bool isStateChanged = (m_curState != state);
        
        if (-1 != m_curState && m_curState != state)
        {
            State& s = m_states[m_curState];

            if (s.onLeave)
                (m_parent->*(s.onLeave))();
        }

        m_preState = m_curState;
        m_curState = state;

        State& s = m_states[state];

        if (0 != s.onEnter)
        {
            (m_parent->*(s.onEnter))();
        }

        m_isLoop         = isLoop;
        m_expireTime     = expireTime;
        m_expireInterval = expireTime;

        if (0 != expireTime)
            m_expireTime += (int)time(0);

        //if (isStateChanged && 0 != m_onStateChanged)
        {
            (m_parent->*(m_onStateChanged))();
        }
    }

    void SetRawState(int state,int expireTime = 0,bool isLoop = false)
    {
        assert(state >= 0 && state < _maxState && "invalid state");

        if (state >= _maxState)
            return;

        if (m_curState == state)
            return;

        m_preState = m_curState;
        m_curState = state;
    
        m_isLoop         = isLoop;
        m_expireTime     = expireTime;
        m_expireInterval = expireTime;

        if (0 != expireTime)
            m_expireTime += (int)time(0);
    }
    
    //获取前面的一个状态
    int GetPreState(void) const {return m_preState;}

    void Update(int curTime)
    {
        if (-1 == m_curState)
            return;

        if (0 != m_expireTime && curTime > m_expireTime)
        {
            State& s = m_states[m_curState];

            //先重新设置超时的时间，然后切换状态
            m_expireTime = 0;

            if (m_isLoop)
                m_expireTime = curTime + m_expireInterval;

            if (0 != s.onExpire)
            {
                (m_parent->*(s.onExpire))();
            }
        }
		else
		{
			State& s = m_states[m_curState];

			if (0 != s.onExec)
			{
				(m_parent->*(s.onExec))();
			}
		}
    }

    void Trigger(void)
    {
        if (m_curState < 0)
            return;
        
        State& s = m_states[m_curState];

        if (0 == s.onTrigle)
            return;
      
        (m_parent->*s.onTrigle)();
    }

    template<typename _p1>
    void Trigger(_p1 p1)
    {
        if (m_curState < 0)
            return;

        State& s = m_states[m_curState];

        if (0 == s.onTrigle)
            return;

        (m_parent->*s.onTrigle)(p1);
    }

    template<typename _p1,typename _p2>
    void Trigger(_p1 p1,_p2 p2)
    {
        if (m_curState < 0)
            return;

        State& s = m_states[m_curState];

        if (0 == s.onTrigle)
            return;

        (m_parent->*s.onTrigle)(p1,p2);
    }

    template<typename _p1,typename _p2,typename _p3>
    void Trigger(_p1 p1,_p2 p2,_p3 p3)
    {
        if (m_curState < 0)
            return;

        State& s = m_states[m_curState];

        if (0 == s.onTrigle)
            return;

        (m_parent->*s.onTrigle)(p1,p2,p3);
    }

    template<typename _p1,typename _p2,typename _p3,typename _p4>
    void Trigger(_p1 p1,_p2 p2,_p3 p3,_p4 p4)
    {
        if (m_curState < 0)
            return;

        State& s = m_states[m_curState];

        if (0 == s.onTrigle)
            return;

        (m_parent->*s.onTrigle)(p1,p2,p3,p4);
    }
    
    int  GetState(void) const {return m_curState < 0 ? 0 : m_curState;}

public:
    _parent* m_parent;
    int      m_preState;
    int      m_curState;
    int      m_expireInterval;
    int      m_expireTime;
    bool     m_isLoop;
    default_handler  m_onStateChanged;
    State    m_states[_maxState];
};

#endif /*_TIMERSTATEMACHINE_H_*/