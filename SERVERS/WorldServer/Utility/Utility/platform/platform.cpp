#include "platform.h"
#include <time.h>
#include "Common/OrgBase.h"
#include <stdio.h>

void Platform::getLocalTime(LocalTime &lt)
{
	struct tm *systime;
	time_t long_time;

	time( &long_time );                // Get time as long integer.
	systime = localtime( &long_time ); // Convert to local time.

	lt.sec      = systime->tm_sec;
	lt.min      = systime->tm_min;
	lt.hour     = systime->tm_hour;
	lt.month    = systime->tm_mon;
	lt.monthday = systime->tm_mday;
	lt.weekday  = systime->tm_wday;
	lt.year     = systime->tm_year;
	lt.yearday  = systime->tm_yday;
	lt.isdst    = systime->tm_isdst;
}

U32 Platform::getTime()
{
	time_t long_time;
	time( &long_time );
	return long_time;
}

void Platform::getDateTime(U32 t, LocalTime &lt)
{
	errno_t err;
	tm systime;

	__time32_t long_time = (__time32_t)t;
	err = _localtime32_s(&systime,&long_time);
	if(err)
		return;

	lt.sec      = systime.tm_sec;
	lt.min      = systime.tm_min;
	lt.hour     = systime.tm_hour;
	lt.month    = systime.tm_mon;
	lt.monthday = systime.tm_mday;
	lt.weekday  = systime.tm_wday;
	lt.year     = systime.tm_year;
	lt.yearday  = systime.tm_yday;
	lt.isdst    = systime.tm_isdst;
}

// ----------------------------------------------------------------------------
// 判断curTime是否在指定时间段内
// 参数1和参数2格式相同，为时间值，如: 093059
bool Platform::isInTime(U32 start, U32 end, U32 current)
{	
	tm t;
	if(_localtime32_s(&t,(__time32_t*)&current))
		return false;

	current = t.tm_hour*10000 + t.tm_min*100 + t.tm_sec;
	return (current >= start && current <= end);
}

// ----------------------------------------------------------------------------
// 判断curTime是否在指定月时内
// 参数1和参数2格式相同，为月日时分秒，如:0429093059 -->四月二十九日9点30分59秒
bool Platform::isInMonthTime(U32 start, U32 end, U32 current)
{
	tm t;
	if(_localtime32_s(&t,(__time32_t*)&current))
		return false;

	current = (t.tm_mon+1)*100000000 + t.tm_mday*1000000 + 
		t.tm_hour* 10000 + t.tm_min*100 + t.tm_sec;
	return (current >= start && current <= end);
}

// ----------------------------------------------------------------------------
// 判断curTime是否在指定期间内
// 参数1和参数3格式相同，为日期值，如：20120411
// 参数2和参数4格式相同，为时间值，如: 093059
bool Platform::isInPeriod(U32 startdate, U32 starttime, U32 enddate, U32 endtime, U32 curTime)
{
	tm t;
	if(_localtime32_s(&t,(__time32_t*)&curTime))
		return false;

	U32 curDate = (t.tm_year+1900)*10000 + (t.tm_mon+1)*100 + t.tm_mday;
	curTime = t.tm_hour* 10000 + t.tm_min*100 + t.tm_sec;
	return (curDate >= startdate && curDate <= enddate) && 
		(curTime >= starttime && curTime <= endtime);
}

// ----------------------------------------------------------------------------
// 获取月份最后一天
S32 Platform::getMonthLastDay(S32 year, S32 month)
{
	static S32 Month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	month %= 12;
	S32 day = Month[month-1];

	if((year%400 == 0) || (year%4 == 0 && year%100 != 0))
	{
		if(month==2)
			day += 1;
	}
	return day;
}

// ----------------------------------------------------------------------------
// 按类型获取特定时间(返回值,如: 1201010000  --->2012年1月1日0点0分
U32 Platform::getNextTime(S32 intervalType)
{
	U32 cur;
	U32 year, month, day;
	tm t;

	_time32((__time32_t*)&cur);
	if(_localtime32_s(&t,(__time32_t*)&cur))
		return 0;

	if(intervalType == INTERVAL_YEAR)
	{
		year = t.tm_year - 100 + 1;
		month = day = 1;
	}
	else if(intervalType == INTERVAL_MONTH)
	{
		year = t.tm_year - 100;
		month = t.tm_mon + 1 + 1;
		if(month >= 13)
		{
			month = 1;
			year++;
		}
		day = 1;
	}
	else if(intervalType == INTERVAL_WEEK)
	{
		if(t.tm_wday != 0)
		{
			cur += ((8-t.tm_wday) * 24 * 3600);
		}
		else
		{
			cur += (24*3600);
		}

		if(_localtime32_s(&t,(__time32_t*)&cur))
			return 0;

		year = t.tm_year - 100;
		month = t.tm_mon + 1;
		day = t.tm_mday;
	}
	else
	{
		cur += (24*3600);
		if(_localtime32_s(&t,(__time32_t*)&cur))
			return 0;

		year = t.tm_year - 100;
		month = t.tm_mon + 1;
		day = t.tm_mday;
	}
	return year * 100000000 + month * 1000000 + day * 10000;
}
