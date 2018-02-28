#ifndef TIMESET_BASE_H
#define TIMESET_BASE_H
#include "Common/PlayerStruct.h"
#include "BASE/types.h"
#include "BASE/base64.h"
#include <string>
	
enum enTimeID
{
	CARD_DEVELOP_TIMER1			= 1,//1-6位大臣派遣CD	
	CARD_DEVELOP_TIMER2,
	CARD_DEVELOP_TIMER3,
	CARD_DEVELOP_TIMER4,
	CARD_DEVELOP_TIMER5,
	CARD_DEVELOP_TIMER6,
	IMPOSE_TIMER				= 10,//每小时产出	
	NEXTHHOUR_TIMER				= 11,//半小时定时器
	NEXTDAY_TIMER				= 12,//隔天定时器
	NEXTHOUR_TIMER				= 13,//每小时定时器
	IMPOSE_TIMER1				= 14,//征收上限
	NEXTTENMINUTES_TIMER		= 15,//每十分钟
	GATE_IMTER					= 20,//20:记录关卡ID	
	APPLY_TIMER1				= 21,//奏章定时器
	APPLY_TIMER2,
	APPLY_TIMER3,
	APPLY_TIMER4,
	APPLY_TIMER5,
	APPLY_TIMER6,
	TURN_CARD_TIMER			    = 30,//翻牌
	DISPATCH_PRINCESS_TIMER1	= 31,//交泰殿
	DISPATCH_PRINCESS_TIMER2,
	DISPATCH_PRINCESS_TIMER3,
	DISPATCH_PRINCESS_TIMER4,
	LEARN_RITE_TIMER1			= 40,//太极殿
	LEARN_RITE_TIMER2,
	LEARN_RITE_TIMER3,
	LEARN_RITE_TIMER4,
	LEARN_RITE_TIMER5,
	PLANT_FLOWER_TIMER1			= 45,//养花
	PLANT_FLOWER_TIMER2,
	PLANT_FLOWER_TIMER3,
	PLANT_FLOWER_TIMER4,
	PLANT_FLOWER_TIMER5,
	PRINCE_SLOT_TIMER			= 55,//皇子席位
	FACE_SET_TIMER				= 60,//头像CD
	BATTLE_TIMER1				= 61,
	BATTLE_TIMER2,
	BATTLE_TIMER3,
	BATTLE_TIMER4,
	BATTLE_TIMER5,
	BATTLE_TIMER6,
	BATTLE_TIMER7,				//67
	TECHNOLOGY_TIMER			= 70,//科技
	TECHNOLOGY_TIMER1,
	TECHNOLOGY_TIMER2,

	CITY_TIMER1					= 75,//寻访
	CITY_TIMER2,
	NEW_PLAYER_TIMER			= 80,//新手标志，新注册用户,隔天清掉
	LOG_LOGIN_TIME				= 81,//登录时长
	SHOP_BUY_TIMER1				= 200,//商城每日限购次数
	SHOP_BUY_TIMER2,
	SHOP_BUY_TIMER3,
	SHOP_BUY_TIMER4,
	SHOP_BUY_TIMER5,
	SHOP_BUY_TIMER6,
	SHOP_BUY_TIMER7,
	SHOP_BUY_TIMER8,
	SHOP_BUY_TIMER9,
	SHOP_BUY_TIMER10,
	SHOP_BUY_TIMER11,
	SHOP_BUY_TIMER12,
	SHOP_BUY_TIMER13,
	SHOP_BUY_TIMER14,
	SHOP_BUY_TIMER15,
	SHOP_BUY_TIMER16,
	SHOP_BUY_TIMER17,
	SHOP_BUY_TIMER18,
	SHOP_BUY_TIMER19,
	SHOP_BUY_TIMER20,

	PVP_CARD_TIMER			   = 220,//用于pvp大臣
	PVP_CARD_TIMER1			   = 221,
	PVP_CARD_TIMER80		   = 300,

	COUNT_TIMER1			   = 350, //用于统计消耗黄金，银两，矿石，士兵，
									  //(废弃),(废弃)
	COUNT_TIMER2,					  //拥有皇子数量，拥有绝世珍宝数目,研究科技累计次数，
									  //升级大臣技能次数，资质累计次数，突破次数
	COUNT_TIMER3,					  //招募1名大臣（含已招募),拥有1名指定爵位的大臣,册封1名妃子达到指定等级,
									  //任意妃子天赋达到指定等级,任意城市开发至指定等级,任意妃子魅力达到指定值
	COUNT_TIMER5,					  //在微服私访中招募1名妃子（含已招募),任意妃子宠幸达到指定值,累积开府皇子开府数量
									  //开府皇子纳贡指定次数,任意大臣达到指定官品,校场积分达到指定值
	COUNT_TIMER6,					  //国力达到指定值,商业达到指定值,工业达到指定值
									  //军事达到指定值,疆土达到指定值,后宫势力达到指定值
	COUNT_TIMER7,					  //累计翻盘指定次数，(废弃)，(废弃)
									  //(废弃),累计宴会指定次数
	COUNT_TIMER8,					  //(累计培养皇子次数，累计平乱指定次数，累计排行榜膜拜次数
	CARD_HIRE_TIMER1			= 370,//征税指定次数，征矿指定次数,征兵指定次数,
									  //寻访指定次数,内政开发达到指定次数
	CARD_HIRE_TIMER2,				  //大臣招募登录天数，登天时间
	DINNER_SHOP_TIME1			= 380,//商城兑换记录(0-5)
	DINNER_SHOP_TIME2,				  //商城兑换记录(6-9)
	DINNER_FREE_TIME,				  //加入免费次数

	TOP_RANK_UP_TIME			= 385,//膜拜次数
	TOP_UP_PRINCESS				= 386,//增幅排行榜,

	ORG_DONATE_TIME				= 390,//联盟捐献

	ACTIVITY_TIME1				= 500,//活动编号,贪官资源
	ACTIVITY_TIME2,					  //鞭刑CD
	ACTIVITY_TIME3,					  //单笔充值奖励1
	ACTIVITY_TIME4,					  //单笔充值奖励2
	ACTIVITY_TIME5,					  //单笔充值奖励3
	ACTIVITY_TIME6,					  //单笔充值奖励4
	ACTIVITY_GOLD_TIME			= 515,//限时黄金消耗
	ACTIVITY_MONEY_TIME,			  //限时银两消耗
	ACTIVITY_ORE_TIME,				  //限时矿石消耗
	ACTIVITY_TROOP_TIME,			  //限时兵力消耗
	ACTIVITY_LOGIN_TIME,			  //限时登陆天数
	ACTIVITY_DINNER_TIME,			  //限时累计宴会次数
	ACTIVITY_IMPOSE_TIME,			  //限时累计征收次数
	ACTIVITY_READAPPLY_TIME,		  //限时累计批阅奏章
	ACTIVITY_ITEM_TIME1,		      //限时翡翠玉石消耗
	ACTIVITY_ITEM_TIME2,		      //限时资质书消耗
	ACTIVITY_ITEM_TIME3,		      //限时技能书消耗
	ACTIVITY_ITEM_TIME4,		      //限时累计宝物消耗
	ACTIVITY_PLANT_TIME,			  //限时累计养花次数
	ACTIVITY_TURNCARD_TIME,			  //限时累计翻牌子次数
	ACTIVITY_PRINCESS_TIME1,		  //限时妃子礼仪培养累计
	ACTIVITY_VISITYCITY_TIME,	      //限时微服私访次数累计
	ACTIVITY_PRINCESS_TIME2,		  //限时处理宫务次数累计
	ACTIVITY_PVP_TIME,				  //限时校场挑战次数累计
	ACTIVITY_PRINCE_TIME,			  //限时培养皇子次数累计

	VIP_TIME					= 550,//VIP礼包领取记录

	NOVICEGUIDE					= 560,//新手引导记录

	PVP_CD_TIME					= 570,//PVP cd 1小时
	PVP_BATTLE_TIMES			= 571,//PVP 战斗次数

	LASTTRIGGERLOGIN_TIME		= 580,//最后登录时间

	DINNER_PRINCESS_TIME1		= 601,//记录妃子是否进行过宴会
	DINNER_PRINCESS_TIME80		= 680,//

	REFLASH_TIME = 681,	//反叛生成

	VIP_INIT_IMPOSE_TIME = 682, //VIP增收令bug更新，累加到增收令上限标志
 };
		

//任务结构
struct stTimeSet
{
	U16		ID;
	U32		PlayerID;
	std::string Flag;//旗帜标志,37个字节
	S32		Flag1;
	S32		ExpireTime;

	stTimeSet():ID(0), PlayerID(0), Flag(""), ExpireTime(0), Flag1(0)
	{
	}

	void WriteData(Base::BitStream* stream)
	{
		stream->writeInt(ID, Base::Bit16);
		stream->writeString(Flag.c_str(), MAX_UUID_LENGTH);
		stream->writeInt(Flag1, Base::Bit32);
		stream->writeInt(ExpireTime, Base::Bit32);
	}
};

struct stTimeSetInfo
{
	stTimeSetInfo() {
		memset(nVal, 0, sizeof(nVal));
	}

	stTimeSetInfo(std::string strFlag) {
		char buf[33];
		memset(buf, 0, sizeof(buf));
		base64_decode(strFlag.c_str(), (unsigned char*)buf, strFlag.length());
		dMemcpy(nVal, buf, sizeof(nVal));
	}

	std::string GetFlag()	{
		char buf[33];
		memset(buf, 0, sizeof(buf));
		dMemcpy(buf, nVal, sizeof(nVal));
		return base64_encode((const unsigned char*)buf, 24);
	}

	S32 nVal[6];
};

typedef std::shared_ptr<stTimeSet> TimeSetRef;
#endif //TIMESET_BASE_H