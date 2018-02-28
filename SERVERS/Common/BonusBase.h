#ifndef _BONUSBASE_H_
#define _BONUSBASE_H_

#define	MAX_PAGE_BONUS_ITEM     12            //每页礼包最大项数
#define MAX_BONUS_ITEM_NUM      10            //每个礼包最多10个物品
#define MAX_ITEMBONUS_COUNT     48            //一次最大发送50条
#define MAX_BONUS_CODESTRING    100           //code最大长度

enum enBonusType
{
	BONUSTYPE_NONE = 0,
	BONUSTYPE_VIP,              // VIP
	BONUSTYPE_ACTIVITY,         // 活动
	BONUSTYPE_SERIAL,           // code领取
	BONUSTYPE_OTHER,            // 特殊
	BONUSTYPE_MAXCOUNT,
};

enum enBonusDB
{
	BONUS_DB_NONE = 0,        // 正常领取
	BONUS_DB_ISGET,           // 已被领取
	BONUS_DB_ONFIND,          // 未找到记录
	BONUS_DB_PLAYERERROR,     // 角色不匹配
	BONUS_DB_UNKNOWN,         // 未知错误
};

enum enReceiveFlag
{
	RECEIVE_CAN = 0,         //可以领取
	RECEIVE_CANNOT,          //不可以领取　　　　
	RECEIVE_END,             //已经领取
};

struct stBonusItem
{
	int            uId;            // 世界编号
	int            itemId;         // 物品Id
	int            receiveFlag;    // 是否收取
	char           szDesc[512];    // 描述
	unsigned char  type;           // 类型
	unsigned short quantity;       // 物品数量

	void copy(const stBonusItem& other)
	{
		memcpy(this,&other,sizeof(stBonusItem));
	}

	stBonusItem()
	{
		memset(this,0,sizeof(stBonusItem));
	}

	stBonusItem(const stBonusItem& other)
	{
		copy(other);
	}

	stBonusItem& operator =(const stBonusItem& other)
	{
		copy(other);
		return *this;
	}

	template<class T>
	void WriteData(T* stream)
	{
		stream->writeBits(sizeof(stBonusItem)<<3,this);
	}

	template<class T>
	void ReadData(T* stream)
	{
		stream->readBits(sizeof(stBonusItem)<<3,this);
	}
};
#endif //_BONUSBASE_H_