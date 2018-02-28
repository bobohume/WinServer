#ifndef TOPRANK_BASE_H
#define TOPRANK_BASE_H
#include "BASE/types.h"
#include "Common/UtilString.h"
#include <string>
#include <memory>

//排行榜
struct stTopRank
{
	std::string UID;
	S8		Type;
	S32		Val[3];//0 value
	S32		LastTime;
	
	//no db
	std::string Name;

	stTopRank() : UID(""), Type(0), LastTime(0), Name("")
	{
		memset(Val, 0, sizeof(Val));
	}

	void WriteData(Base::BitStream* stream, int order)
	{
		stream->writeString(Util::MbcsToUtf8(Name), 50);
		stream->writeInt(order, Base::Bit32);
		stream->writeInt(Val[0], Base::Bit32);
	}

	void WriteDataEx(Base::BitStream* stream);
	void WriteDataEx(Base::BitStream* stream, int order);
};

typedef std::shared_ptr<stTopRank> TopRankRef;

enum TopRankType
{
	eTopType_Start = 0,
	eTopType_PVE = eTopType_Start,
	eTopType_PVEWIN,
	eTopType_Battle,
	eTopType_Card,
	eTopType_Princess,
	eTopType_Land,

	eTopType_BattleUp,
	eTopType_CardUp,
	eTopType_PrincessUp,
	eTopType_LandUp,

	//总数
	eTopType_End
};

//文字描述
static const char* strTopRankType[eTopType_End] =
{
	"pve",					//
	"pve连胜表",					//
	"国力榜",
	"大臣榜",
	"妃子榜",
	"疆土榜",
	"国力榜涨幅",
	"大臣榜涨幅",
	"妃子榜涨幅",
	"疆土榜涨幅",
};


#define TR_SORTTIME (60)        //秒
#define TR_TOPCOUNT (2000)
#define TR_CLEARTIME (21*24*60*60)  //21天不上线清排行榜

//排行榜的配置
struct TopRankConfig
{
	TopRankConfig(void)
	{
		sortInterval = TR_SORTTIME;
		playerCount = TR_TOPCOUNT;
		lastSortTime = (U32)time(0);
	}

	//排队的玩家数
	U32 playerCount;
	//排序刷新时间
	U32 sortInterval;
	//备份的数据
	time_t lastSortTime;
};

extern TopRankConfig gs_topRankConfigs[eTopType_End];
#endif //TOPRANK_BASE_H