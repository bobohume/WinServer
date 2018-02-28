#ifndef _TIMERANGE_H_
#define _TIMERANGE_H_

class TimeRange
{
public:
    TimeRange(void);

    bool InitFromString(const char* timeRange,bool everyDayLimit);
    
    bool IsInRange(time_t curTime);
protected:
    int  m_sy;
    int  m_sm;
    int  m_sd;
    int  m_ey;
    int  m_em;
    int  m_ed;
    int  m_sh;
    int  m_smin;
    int  m_ss;
    int  m_eh;
    int  m_emin;
    int  m_es;

    bool m_isEveryDayLimit;
};

#endif /*_TIMERANGE_H_*/