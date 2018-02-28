#ifndef TOP_TYPE_H
#define TOP_TYPE_H

#include <string>
#include "time.h"
#include "PlayerStruct.h"

//副本最多不超过一天
#define RANK(a,b) ((10-(a))*100000+(b))
#define GETFIRST(a) (10-(a)/100000)
#define GETSECOND(a) (a)%100000
#define SPC_TYPE	999999999999999
//排行榜类型,序号不可改
enum TopRankType
{
	eTopType_Start				= 0,
	//统计
	eTopType_Level				= eTopType_Start,					// 等级
	eTopType_Rich				= 1,								// 金钱
	eTopType_Score				= 2,								// 积分

	//总数
	eTopType_End
};

enum TopRankBlobType
{
	ePetBlob = 0,
	eItemBlob
};

//文字描述
static const char* strTopRankType[eTopType_End] =
{
	"Level",					// 0-等级
	"Rich",						// 1-金钱
	"Score",					// 2-积分
};

#define TR_SORTTIME (60)        //秒
#define TR_SAVETIME (60 * 60)
#define TR_TOPCOUNT (99)
#define TR_ARENACOUNT (200)     //竞技场保存的个数
#define TR_CLEARTIME (21*24*60*60)  //21天不上线清排行榜

//排行榜的配置
struct TopRankConfig
{     
    TopRankConfig(void)
    {
		sortInterval	= TR_SORTTIME;
		saveInterval	= TR_SAVETIME;
		playerCount		= TR_TOPCOUNT;
		lastSortTime	= (U32)time(0);
		lastSaveTime	= (U32)time(0);
    }
    
	//排序刷新时间
	U32 sortInterval;

	//数据库保存时间
	U32 saveInterval;

	//排队的玩家数
	U32 playerCount;   
    
    //备份的数据
	time_t lastSortTime;
	time_t lastSaveTime;
};

extern TopRankConfig gs_topRankConfigs[eTopType_End];

//struct PlayerEquip
//{
//	stItemInfo	EquipInfo[12];
//	U8			Sex;
//	U8			Level;
//	U8			Family;
//	char		PlayerName[32];
//
//	PlayerEquip()
//	{
//		memset(this,0,sizeof(PlayerEquip));
//	}
//
//	void packData(Base::BitStream& packet,stPlayerStruct pPlayer)
//	{
//		for (int i = 1; i < 13; ++i)   //普通装备1-12件装备
//		{
//			if(pPlayer.MainData.EquipInfo[i].ItemID != 0)
//			{
//				packet.writeFlag(true);
//				pPlayer.MainData.EquipInfo[i].WriteData(&packet);
//			}
//			else
//				packet.writeFlag(false);
//		}
//		packet.writeInt(pPlayer.BaseData.Sex,8);
//		packet.writeInt(pPlayer.BaseData.Level,8);
//		packet.writeInt(pPlayer.BaseData.Family,8);
//		packet.writeString(pPlayer.BaseData.PlayerName,32);
//	}
//
//	void unpackData(Base::BitStream& packet)
//	{
//		for (int i = 1; i < 13; ++i)   //普通装备1-12件装备
//		{
//			if (packet.readFlag())
//				EquipInfo[i].ReadData(&packet);		
//		}
//		Sex			= packet.readInt(8);
//		Level		= packet.readInt(8);
//		Family		= packet.readInt(8);
//		packet.readString(PlayerName,32);
//
//	}
//};


//排行榜的数据项
struct TopRankItem
{
	U64 uid;				//唯一标识,可以是玩家或物品或宠物ID
	int playerId;			//玩家Id
	int idxValue;			//排名升降
	int lastRank;			//上次的排名
	int orgLV;				//家族等级，玩家等级(竞技场)
	int nValue;				//排序值
	int honour;				//玩家职业(主键是玩家)，异兽等级(主键是灵兽)，奇珍类型(主键装备)
	U64 exp;				//经验  
	time_t lastTime;        //上次更新的时间
	std::string strName;	//名称
	std::string itemName;	//奇珍或者异兽名称

	stPetInfo		petInfo;
	stItemInfo		ItemInfo; 
    
    TopRankItem(void)
    {
        memset(this,0,sizeof(TopRankItem));
        lastRank = -1;
    }

	bool operator < (const TopRankItem& other) const
	{	
		if(nValue == other.nValue)
		{
// 			if (exp == other.exp)
// 			{
			
				if(SPC_TYPE == exp && SPC_TYPE == other.exp) // 为Value相同却需要共存的时候-初始化使用的初始值exp=999999999999999来增加新的排序规则
					return uid < other.uid;
				else if (-1 == lastRank)  //-1为降序未上榜数据，-2为升序未上榜数据,
					return true;
				else if(-2 == lastRank)  
					return false;
				else
					return true;
// 			}
// 			return exp < other.exp;
		}
			

		return nValue < other.nValue;
	}
    
    void packData(Base::BitStream& packet)
    {
       packet.writeInt(playerId, 32);
		packet.writeInt(nValue, 32);
		packet.writeInt(honour, 32);
		packet.writeInt(orgLV,32);
		packet.writeSignedInt(idxValue,16);
		packet.writeSignedInt(lastRank, 16);
		packet.writeInt((S32)lastTime,32);
		packet.writeString(strName.c_str());
		packet.writeString(itemName.c_str());
		char buf[64] = "";
		_i64toa_s(uid,buf,64,10);
		packet.writeString(buf);
    }
    
	void unpackData(Base::BitStream& packet)
	{
		playerId = packet.readInt(32);
		nValue   = packet.readInt(32);
		honour   = packet.readInt(32);
		orgLV	 = packet.readInt(32);
		idxValue = packet.readSignedInt(16);
		lastRank = packet.readSignedInt(16);
		lastTime = packet.readInt(32);

		char nameBuf[256] = {0};
		packet.readString(nameBuf,sizeof(nameBuf));
		strName  = nameBuf;
		packet.readString(nameBuf,sizeof(nameBuf));
		itemName = nameBuf;
		packet.readString(nameBuf,sizeof(nameBuf));
		uid = _atoi64(nameBuf);

	}
};

#endif