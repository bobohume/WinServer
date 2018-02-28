#ifndef _SIMPLETIMER_H_
#define _SIMPLETIMER_H_

#include "CommLib.h"
#include <time.h>

template<int _Interval>
class SimpleTimer
{
public:
	SimpleTimer(bool isActived = true)
	{
		m_interval = _Interval;
		m_lastTime = 0;
		m_count    = 0;
        m_isActive = isActived;
	}

	void SetInterval(U32 interval) {m_interval = interval;}

	bool CheckTimer(void)
	{
        if (!m_isActive)
            return false;

		U32 curTime = (U32)_time32(0);

		if (curTime - m_lastTime >= m_interval)
		{
			m_lastTime = curTime;
			m_count++;
			return true;
		}

		return false;
	}

	void Start(void)
	{
		m_lastTime = (U32)_time32(0);
		m_count    = 0;
        m_isActive = true;
	}

    void Stop(void)
    {
        m_lastTime = (U32)_time32(0);
		m_count    = 0;
        m_isActive = false;
    }
		
	//触发的次数
	U32 GetTimerCount(void) const {return m_count;}

	U32 GetLastTime(void) const 
	{
        if (!m_isActive)
            return 0;

		U32 curTime = (U32)_time32(0);
		return curTime - m_lastTime;
	}

    bool IsActived(void) const {return m_isActive;}
protected:
	U32  m_interval;
	U32  m_lastTime;
	U32  m_count;
    bool m_isActive;
};

#endif /*_SIMPLETIMER_H_*/