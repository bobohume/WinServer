//
//  NpcData.h
//  HelloLua
//
//  Created by BaoGuo on 14-1-25.
//
//
#ifndef HelloLua_MissionData_h
#define HelloLua_MissionData_h
#include "Common/DataFile.h"
#include <string.h>
#include <hash_map>
#include <set>

class CMissionData;
class CMissionDataRes
{
public:
	typedef stdext::hash_multimap<S32, S32> MISSIONIDMAP;
	typedef MISSIONIDMAP::iterator MISSIONIDITR;

	enum{
		MISSION_BEGIN_ID = 700001,
		MISSION_END_ID   = 799999,
	};

	typedef stdext::hash_map<S32, CMissionData*> MISSIONDATAMAP;
	typedef MISSIONDATAMAP::iterator MISSIONDATAITR;
	CMissionDataRes();
	~CMissionDataRes();
	bool read();
	void close();
	void clear();
	CMissionData* getMissionData(S32 iMissionId);
	static CMissionData* getMissionDataEx(S32 iMissionId);
	//获取下个任务
	bool  getNextMissionId(S32 iPreMissionId, std::set<S32>& MissionSet);

private:
	MISSIONDATAMAP m_MissionDataMap;
	MISSIONIDMAP   m_MissisonIdMap;
};

class CMissionData
{
public:
	enum enMissionCategory
	{
		MISSION_CATEGORY_PLOT           =  1,//剧情
		MISSION_CATEGORY_ACTIVITY       =  2,//活动
		MISSION_CATEGORY_GROWUP         =  3,//成长
		MISSION_CATEGORY_GATE           =  4,//关卡
		MISSION_CATEGORY_COMBAT         =  5,//战斗
		MISSION_CATEGORY_ITEM           =  6,//物品
		MISSION_CATEGORY_ORG            =  7,//工会
		MISSION_CATEGORY_DAILY          =  8,//日常
		MISSION_CATEGORY_LOGIN          =  9,//连登
		MISSION_CATEGORY_BANG			=  10,//帮会
	};

	enum enSubMissionCategory
	{
		MISSION_SUBCATEGORY_PRINCIPAL_LINE      = 101,
		MISSION_SUBCATEGORY_BRANCH_LINE         = 102,
		MISSION_SUBCATEGORY_SPECIAL             = 103,
		MISSION_SUBCATEGORY_TIMELIMIT           = 201,
		MISSION_SUBCATEGORY_COUNTLIMIT          = 202,
		MISSION_SUBCATEGORY_LEVELLIMIT          = 203,
		MISSION_SUBCATEGORY_LEVEL               = 301,
		MISSION_SUBCATEGORY_PRO                 = 302,
		MISSION_SUBCATEGORY_NEWPALYERGATE       = 401,
		MISSION_SUBCATEGORY_MISSIONNAME         = 402,
		MISSION_SUBCATEGORY_YUEYOUJING			= 403,//月幽境
		MISSION_SUBCATEGORY_LUOXIYUAN			= 404,//落夕渊
		MISSION_SUBCATEGORY_SHENMULIN			= 405,//神木林
		MISSION_SUBCATEGORY_SHUANGSHENGSHAN		= 406,//双生山
		MISSION_SUBCATEGORY_XITIAN				= 407,//西天
		MISSION_SUBCATEGORY_PENGLAI				= 408,//蓬莱
		MISSION_SUBCATEGORY_BINGCHI				= 410,//醉梦冰池
		MISSION_SUBCATEGORY_KUNLUN				= 411,//昆仑古墟
		MISSION_SUBCATEGORY_WANSHULOU			= 412,//万书楼
		MISSION_SUBCATEGORY_TIANSHUTA			= 413,//天书塔
		MISSION_SUBCATEGORY_DOUXIAN				= 414,//斗仙玄境
		MISSION_SUBCATEGORY_XINMO				= 415,//心魔玄境
		MISSION_SUBCATEGORY_TIANGUAN			= 416,//无极天关
		MISSION_SUBCATEGORY_DUOBAO				= 417,//降妖夺宝
		MISSION_SUBCATEGORY_WANLING				= 418,//万灵之战
		MISSION_SUBCATEGORY_BAIHUA				= 419,//百花秘境
		MISSION_SUBCATEGORY_YUNXIANZHEN 		= 420,//陨仙阵
		MISSION_SUBCATEGORY_LONGWNAG			= 421,//龙王宝洞
		MISSION_SUBCATEGORY_JIUXIAN				= 422,//酒仙迷境
		MISSION_SUBCATEGORY_YZCHENGBAO			= 423,//云中城堡
		MISSION_SUBCATEGORY_XHQIJU				= 424,//星瀚棋局
		MISSION_SUBCATEGORY_TMSHILIAN		    = 425,//天门试炼
		MISSION_SUBCATEGORY_DNTIANGONG			= 426,//大闹天宫
		MISSION_SUBCATEGORY_HAOTIANTA			= 427, //昊天塔
		MISSION_SUBCATEGORY_JGQINGYUAN			= 428,//九宫情缘
		MISSION_SUBCATEGORY_KILLNUM             = 501,
		MISSION_SUBCATEGORY_COLLECT             = 601,
		MISSION_SUBCATEGORY_HECHENG				= 602,//合成
		MISSION_SUBCATEGORY_LEVEL_ORG           = 701,
		MISSION_SUBCATEGORY_TIMELIMIT_DIALY     = 801,
		MISSION_SUBCATEGORY_COUNTLIMIT_DIALY    = 802,
		MISSION_SUBCATEGORY_LEVELLIMIT_DIALY    = 803,
		MISSION_SUBCATEGORY_ZHANDOU				= 804,//战斗
		MISSION_SUBCATEGORY_LOGIN_ONE           = 901,
		MISSION_SUBCATEGORY_LOGIN_TWO           = 902,
		MISSION_SUBCATEGORY_LOGIN_THREE         = 903,
		MISSION_SUBCATEGORY_LOGIN_FOUR          = 904,
		MISSION_SUBCATEGORY_LOGIN_FIVE          = 905,
		MISSION_SUBCATEGORY_LOGIN_SIX           = 906,
		MISSION_SUBCATEGORY_LOGIN_SEVEN         = 907,
		MISSION_SUBCATEGORY_JIUZHU				= 1001, //救助
		MISSION_SUBCATEGORY_BEIJIU				= 1002,//被救
		MISSION_SUBCATEGORY_ZHUZHAN				= 1003,//助战
	};

	enum enCompleteType
	{
		COMPLETE_TYPE_LOGIN                     = 1,
		COMPLETE_TYPE_KILL                      = 2,
		COMPLETE_TYPE_GETITEM                   = 3,
		COMPLETE_TYPE_USE                       = 4,
		COMPLETE_TYPE_STRENGTHEN                = 5,
		COMPLETE_TYPE_COMPOSE                   = 6,
		COMPLETE_TYPE_JIUREN					= 7,//救人
		COMPLETE_TYPE_BEIJIU					= 8,//被救
		COMPLETE_TYPE_HELPBEAT					= 9,//帮助作战
		COMPLETE_TYPE_CHUANGGUAN				= 10,//闯关
		COMPLETE_TYPE_XIOAFEI					= 11,//消费
		COMPLETE_TYPE_KILL_MAN                  = 101,
		COMPLETE_TYPE_KILL_SHENG                = 102,
		COMPLETE_TYPE_KILL_FO                   = 103,
		COMPLETE_TYPE_KILL_XIAN                 = 104,
		COMPLETE_TYPE_KILL_JIN                  = 105,
		COMPLETE_TYPE_KILL_GUI                  = 106,
		COMPLETE_TYPE_KILL_GUAI                 = 107,
		COMPLETE_TYPE_KILL_YAO                  = 108,
		COMPLETE_TYPE_KILL_MO                   = 109,
		COMPLETE_TYPE_KILL_SHOU                 = 110,
		COMPLETE_TYPE_KILL_LONG                 = 111,
		COMPLETE_TYPE_KILL_NORMAL               = 201,
		COMPLETE_TYPE_KILL_ELITE                = 202,
		COMPLETE_TYPE_KILL_BOSS                 = 203,
		COMPLETE_TYPE_KILL_SPECIAL_MONSTER		= 301,//杀死制定怪物
		COMPLETE_TYPE_LEVEL_UP					= 401,//等级提升
		COMPLETE_TYPE_ATKVALUE_GOTO				= 402,//攻击值达到
		COMPLETE_TYPE_HUJIAVALUE_GOTO			= 403,//护甲值达到
	};

	enum enTimeSetInt
	{
		INTERVAL_DAY							= 1,//每天
		INTERVAL_WEEK							= 2,//每周
		INTERVAL_MONTH							= 3,//每月
		INTERVAL_YEAR							= 4,//每年
		INTERVAL_LOOP							= 5,//循环
		INTERVAL_NEVER							= 6,//永不
		INTERVAL_LIANXU							= 7 //连续
	};


	CMissionData():m_MissionId(0), m_Category(0), m_SubCategory(0), m_PreMissionId(0), m_MissionName(""), m_RecLevel(0), m_SubLevel(0), \
		m_WeekDay(0), m_BeginTime(""), m_LastTime(0), m_RecNpcId(0), m_SubNpcId(0), m_MissionType(0), m_TargetId(0), m_NeedNum(0),\
		m_AwardExp(0), m_AwardGold(0),m_AwardItemId(0), m_AwardMoney(0), m_Interval(0), m_MissionDesc(""), m_Points(0),m_AwardScore(0)
	{
	}
	~CMissionData() {};
	S32 m_MissionId;
	U16 m_Category;//大类
	U16 m_SubCategory;//子类
	S32 m_PreMissionId; //前置任务
	std::string m_MissionName; //名称
	U32 m_RecLevel;//接任务等级
	U32 m_SubLevel;//交任务等级
	U32 m_WeekDay;//星期
	std::string m_BeginTime;//开启时间
	U32 m_LastTime;//持续时间
	U32 m_RecNpcId; //起始NPC
	U32 m_SubNpcId; //结束NPC
	U16 m_MissionType;//完成类型
	S32 m_TargetId;//目标
	S32 m_NeedNum;//数量
	S32 m_AwardExp; //经验
	S32 m_AwardMoney; //金元
	S32 m_AwardGold;//元宝
	S32 m_Points;// 点数
	
	S32 m_AwardItemId;//道具
	S32 m_AwardScore;//成就点
	U16 m_Interval;//重置
	std::string m_MissionDesc; //描述

	void printAllEmlem();
};

extern CMissionDataRes* g_MissionDataMgr;
#endif
