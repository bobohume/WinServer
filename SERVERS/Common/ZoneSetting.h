#ifndef __ZONESETTING_H__
#define __ZONESETTING_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"

#define MAX_RESERVE_FIELDS 6

enum RESERVE
{
	DEFAULT1 = 0,						//目前是老玩家活动开关
	DEFAULT2,							//目前是vip网吧活动开关
	DEFAULT3,						//vip帐户活动
	DEFAULT4,        //老玩家和vip帐户活动
	DEFAULT5,
	DEFAULT6,
	DEFAULT7,
	DEFAULT8,
	MAX_WORLDSETS
};

struct stZoneSetting
{
    stZoneSetting(void)
    {
        memset(this,0,sizeof(stZoneSetting));
    }
    
    bool isApplied;
    bool isChanged;
    
	U8	lineid;					//线ID
	U32 zoneid;					//地图编号
	S32 exprate;				//经验倍率
	S32 droprate;				//掉率
    __time64_t timeStart;       //开始时间
    __time64_t timeEnd;         //结束时间
	S32 reserve[MAX_RESERVE_FIELDS];	//保留字段5个
};

//全服游戏逻辑数据配置项
struct stWorldSetting
{
	U8  Years5Assure;			//"五年包养活动"
	U8  GodGivePath;			//"仙人指路活动"
	U32 TopLevel;				//全服玩家最高等级
	U32 TopDisaster;			//天劫最高等级
	U32	ActivityFlag;			//运营设计的活动标志,目前用作"同一物理机登陆次数奖励"活动
	U32 MaxStalls;				//当前商铺总数
	U32 TotalFlourish;			//商业指数(商铺总繁荣度)
	S32 reserve	[MAX_WORLDSETS];
};

#endif//__ZONESETTING_H__
