#include "stdafx.h"
#include "CommLib.h"

#include "TimeRange.h"
#include <atltime.h>

TimeRange::TimeRange(void)
{
    memset(this,0,sizeof(TimeRange));
}

bool TimeRange::InitFromString(const char* timeRange,bool everyDayLimit)
{
    if (0 == timeRange || 0 == strlen(timeRange))
        return false;

    char timeBuf[512] = {0};
    strcpy_s(timeBuf,sizeof(timeBuf),timeRange);

    char startDay[64]  = {0};
    char endDay[64]    = {0};
    char fromTime[64]  = {0};
    char endTime[64]   = {0};

    for (size_t i = 0; i < strlen(timeBuf); ++i)
    {
        if (timeBuf[i] == '|')
        {
            timeBuf[i] = ' ';
        }
    }
    
    if (everyDayLimit)
    {
        sscanf_s(timeBuf,"%s %s %s %s",startDay,sizeof(startDay),endDay,sizeof(endDay),fromTime,sizeof(fromTime),endTime,sizeof(endTime));
    }
    else
    {
        sscanf_s(timeBuf,"%s %s %s %s",startDay,sizeof(startDay),fromTime,sizeof(fromTime),endDay,sizeof(endDay),endTime,sizeof(endTime));
    }

    if (0 == strlen(startDay) ||
        0 == strlen(endDay) ||
        0 == strlen(fromTime) ||
        0 == strlen(endTime))
    {
        return false;
    }

    int rtn = 0;

    rtn = sscanf_s(startDay,"%d-%d-%d",&m_sy,&m_sm,&m_sd);
    
    if (3 != rtn)
        return false;

    rtn = sscanf_s(endDay,"%d-%d-%d",&m_ey,&m_em,&m_ed);

    if (3 != rtn)
        return false;
    
    rtn = sscanf_s(fromTime,"%d:%d:%d",&m_sh,&m_smin,&m_ss);

    if (3 != rtn)
        return false;

    rtn = sscanf_s(endTime,"%d:%d:%d",&m_eh,&m_emin,&m_es);

    if (3 != rtn)
        return false;

    //检测时间是否正确
    if(everyDayLimit)
    {   
        {
            CTime startTimeVal(m_sy,m_sm,m_sd,0,0,0);
            CTime endTimeVal(m_ey,m_em,m_ed,0,0,0);

            if (startTimeVal > endTimeVal)
            {
                return false;
            }
        }
        
        {
            CTime startTimeVal(m_sy,m_sm,m_sd,m_sh,m_smin,m_ss);
            CTime endTimeVal(m_sy,m_sm,m_sd,m_eh,m_emin,m_es);

            if (startTimeVal > endTimeVal)
            {
                return false;
            }
        }
    }
    else
    {
        CTime startTimeVal(m_sy,m_sm,m_sd,m_sh,m_smin,m_ss);
        CTime endTimeVal(m_ey,m_em,m_ed,m_eh,m_emin,m_es);

        if (startTimeVal > endTimeVal)
        {
            return false;
        }
    }

    m_isEveryDayLimit = everyDayLimit;
    return true;
}

bool TimeRange::IsInRange(time_t curTime)
{
    if (m_isEveryDayLimit)
    {
        CTime curTimeVal(curTime);

        {
            CTime startTimeVal(m_sy,m_sm,m_sd,0,0,0);
            CTime endTimeVal(m_ey,m_em,m_ed,23,59,59);

            if (curTime < startTimeVal.GetTime() || curTime > endTimeVal.GetTime())
            {
                return false;
            }
        }

        {
            CTime startTimeVal(curTimeVal.GetYear(),curTimeVal.GetMonth(),curTimeVal.GetDay(),m_sh,m_smin,m_ss);
            CTime endTimeVal(curTimeVal.GetYear(),curTimeVal.GetMonth(),curTimeVal.GetDay(),m_eh,m_emin,m_es);

            if (curTime >= startTimeVal.GetTime() &&
                curTime <= endTimeVal.GetTime())
            {
                return true;
            }
        }
    }
    else
    {
        CTime startTimeVal(m_sy,m_sm,m_sd,m_sh,m_smin,m_ss);
        CTime endTimeVal(m_ey,m_em,m_ed,m_eh,m_emin,m_es);

        if (curTime >= startTimeVal.GetTime() &&
            curTime <= endTimeVal.GetTime())
        {
            return true;
        }
    }

    return false;
}