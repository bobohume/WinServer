#ifndef __AUCTIONBASE_H__
#define __AUCTIONBASE_H__

#include "Base/BitStream.h"
#include "Common/PacketType.h"
#include "common/PlayerStruct.h"
#include "CommLib/CommLib.h"
#include <map>
#include <hash_map>
class ItemShortcut;

enum enAuction
{
	MAX_AUCTION_QUERY	= 200,			//一次Pack给Zone的拍卖行数据最大个数
	MAX_AUCTION_TAX		= 20,			//拍卖行手续费为交易所得元宝的20%
	MAX_AUCTION_SLOTS	= 7,			//拍卖行商品浏览每页可显示的商品数
	MAX_AUCTION_QUERYINTERVAL = 5000,	//拍卖行查询间隔

	AUCTION_BROWSE_0	= 0,			//拍卖商品浏览栏
	AUCTION_BROWSE_1	= 1,			//拍卖商品寄售历史栏
};

enum enAuctionState
{
	AUCSTATE_NONE		= 0,			//拍卖中
	AUCSTATE_CANCEL		= 10,			//拍卖商品被卖主取消
	AUCSTATE_EXPIRY		= 11,			//拍卖商品过期流拍
	AUCSTATE_BUYOK		= 13,			//拍卖商品被购买成功下架
};

enum enAuctionTime
{
	AUCTIONTIME_12,			//12小时
	AUCTIONTIME_24,			//24小时
	AUCTIONTIME_36,			//36小时
	AUCTIONTIME_48,			//48小时
	AUCTIONTIME_MAX,
};

// ----------------------------------------------------------------------------
// 计算保管费的公式
inline S32 calcDeposit(U32 timeType, S32 fixedPrice)
{
	return mClamp((S32)(fixedPrice/100.00f * (timeType+1)), 1000,1000000);
}

#pragma pack(push,1)
// ----------------------------------------------------------------------------
// 拍卖行基本数据
struct stAuctionInfo
{
	U8  Status;				//拍卖行商品状态
	U32 Id;					//拍卖单ID
	U32	Owner;				//拍卖者(Note: 也可是黑市最近竞价者)
	S32 FixedPrice;			//拍卖价格(Note: 也可以黑市最近竞价价格)
	U32	ExpiryTime;			//过期时间
	U32 ItemId;				//物品ID
	S32 ItemNum;			//物品数量
	stItemInfo* pItem;		//物品属性

	stAuctionInfo():Id(0),Owner(0),Status(AUCSTATE_NONE),FixedPrice(0),
		ItemId(0),ItemNum(1),ExpiryTime(0),pItem(NULL)
	{
	}

	~stAuctionInfo() {if(pItem) delete pItem;}
	bool canClear() { return (Status >= AUCSTATE_CANCEL);}
};

//拍卖行物品
struct stAuctionGoods
{
	U32 Id;							// 拍卖行商品ID
	U32 ExpiryTime;					// 保管过期时间
	S32 FixedPrice;					// 元宝价格
	U32 ItemId;						// 物品ID
	S32 ItemNum;					// 物品数量
	ItemShortcut* pItem;			// 物品实例
	stAuctionGoods():Id(0),ExpiryTime(0),
		FixedPrice(0),ItemId(0),ItemNum(0),pItem(NULL) {}
};

#pragma pack(pop)

enum enAuctionNetOp
{
	AUCTION_NET_OPEN,				// 打开拍卖行
	AUCTION_NET_CLOSE,				// 关闭拍卖行
	AUCTION_NET_SETTING,			// 设置寄售上架信息
	AUCTION_NET_BUY,				// 购买拍卖行商品
	AUCTION_NET_BID,				// 竞拍拍卖行商品
	AUCTION_NET_CANCLE,				// 取消寄售
	AUCTION_NET_BROWSE,				// 浏览拍卖行商品
	AUCTION_NET_BROWSEHISTORY,		// 浏览拍卖行历史
	AUCTION_NET_BROWSEPAGE,			// 浏览拍卖行商品翻页
	AUCTION_NET_CLEARTEMP,			// 请求清空寄售设置栏的物品
};

enum enAuctionType
{
	AUCTIONTYPE_SYSTEM,		//官方竞拍
	AUCTIONTYPE_WEAPON,		//武器
	AUCTIONTYPE_EQUIP,		//装备
	AUCTIONTYPE_PET,		//灵兽仙骑
	AUCTIONTYPE_BAOSHI,		//百宝灵石
	AUCTIONTYPE_XIAOHAO,	//日常消耗
	AUCTIONTYPE_OTHER,		//其它

	ASUB_SYSTEM_ITEM			= 1,			//官方拍卖-物品

	ASUB_WEAPON_FAQI			= 10,			//武器-法器
	ASUB_WEAPON_DAOJIAN			= 11,			//武器-刀剑
	ASUB_WEAPON_HUANREN			= 12,			//武器-环刃
	ASUB_WEAPON_QIN				= 13,			//武器-琴
	ASUB_WEAPON_BOW				= 14,			//武器-弓
	ASUB_WEAPON_FLAGSTAFF		= 15,			//武器-旗杖
	ASUB_WEAPON_ARMS			= 16,			//武器-刀斧

	ASUB_EQUIP_HEAD				= 20,			//装备-头部
	ASUB_EQUIP_BODY				= 21,			//装备-身体
	ASUB_EQUIP_BACK				= 22,			//装备-背部
	ASUB_EQUIP_SHOULDER			= 23,			//装备-肩部
	ASUB_EQUIP_HAND				= 24,			//装备-手部
	ASUB_EQUIP_WAIST			= 25,			//装备-腰部
	ASUB_EQUIP_FOOT				= 26,			//装备-脚部
	ASUB_EQUIP_AMULET			= 27,			//装备-护身符
	ASUB_EQUIP_NECK				= 28,			//装备-项链
	ASUB_EQUIP_RING				= 29,			//装备-戒指

	ASUB_PET_PET				= 30,			//灵兽仙骑-灵兽
	ASUB_PET_MOUNT				= 31,			//灵兽仙骑-骑宠
	ASUB_PET_EQUIP				= 32,			//灵兽仙骑-灵兽装备
	ASUB_PET_SKILLBOOK			= 33,			//灵兽仙骑-灵兽技能书

	ASUB_BAOSHI_CHIP			= 40,			//百宝灵石-宝石碎片
	ASUB_BAOSHI_RED				= 41,			//百宝灵石-红宝石
	ASUB_BAOSHI_GREEN			= 42,			//百宝灵石-绿宝石
	ASUB_BAOSHI_BLUE			= 43,			//百宝灵石-蓝宝石
	ASUB_BAOSHI_CRYSTAL			= 44,			//百宝灵石-水晶石
	ASUB_BAOSHI_BOX				= 45,			//百宝灵石-宝石盒

	ASUB_XIAOHAO_MEDIA			= 50,			//日常消耗-丹药食物
	ASUB_XIAOHAO_SCROLL			= 51,			//日常消耗-卷轴符纸
	ASUB_XIAOHAO_BAG			= 52,			//日常消耗-空间扩展
	ASUB_XIAOHAO_EQUIP			= 53,			//日常消耗-装备强化材料
	ASUB_XIAOHAO_PET			= 54,			//日常消耗-灵兽强化材料
	ASUB_XIAOHAO_SPIRIT			= 55,			//日常消耗-元神强化材料
	ASUB_XIAOHAO_SPIRITADD		= 56,			//日常消耗-神元合成材料

	ASUB_OTHER_OTHER			= 60,			//其它-其它
};

//顺序列表（以AuctionManage维护的自增值）
typedef std::map<U32, U32> AUCTIONSIDMAP;
//拍卖物列表
typedef stdext::hash_map<U32, stAuctionInfo*> AUCTIONINFOMAP;
typedef std::list<stAuctionGoods*> LISTAUCTION;

typedef std::vector<stAuctionInfo*> AUCTIONLIST;

#endif//__AUCTIONBASE_H__