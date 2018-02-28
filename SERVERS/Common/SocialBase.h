#ifndef SOCIAL_BASE_H
#define SOCIAL_BASE_H
#include "PlayerStructEx.h"
#include <map>

namespace SocialType
{
	enum Type
	{
		Temp,			// 临时
		Friend,			// 好友
		Consort,		// 配偶
		Master,			// 师傅
		Prentice,		// 徒弟	
		Buddie,			// 兄弟
		Enemy,			// 仇人
		Mute,			// 屏蔽(黑名单)
		Engage,			// 订婚
		Divorce,		// 离婚

		Count,
	} ;

    static const char* strType[Count] = 
    {
        "临时",
        "仙友",
        "仙侣",
        "师傅",
        "徒弟",
        "结拜",
        "仇人",
        "屏蔽",
		"订婚",
		"仙侣",
    };
} ;

static int SocialTypeMaxCount[ SocialType::Count ] = 
{
	50,				// 临时
	50,			    // 仙友
	1,				// 仙侣
	1,				// 师傅
	5,				// 徒弟
	5,				// 结拜
	20,				// 仇人
	100,			// 屏蔽
	1,              // 订婚 
	1,				// 离婚
} ;

enum enSocialError
{
	SocialError_None,
	SocialError_Unknown,		// 未知
	SocialError_Self,			// 不能和自身成为社会关系
	SocialError_MaxCount,		// 此社会关系人数达到最大上限
	SocialError_NotFound,		// 目标玩家不存在
	SocialError_Existed,		// 目标玩家已是此在社会关系列表中
	SocialError_Unallowed,      // 该操作不允许
	SocialError_DbError,        // 数据库操作错误
	
	SocialError_Count,
};

enum enFriendValueType
{
	FriendValue_Non,          // 正常友好度
	FriendValue_Kill,         // 杀怪友好度
	FriendValue_Mission,      // 任务友好度
};

enum enQueryPlayerInfo
{
	QueryTypePlayer,            // 查询玩家数据
	QueryTypeFriend,            // 查询好友数据
};

enum enLimintFriendValue
{
	LimintFriendValue_Kill = 300,    //杀怪上限
	LimintFriendValue_Mission = 200, //任务上限
};

enum enSocialConstants
{
	AddFriendnotify,            // 加好友通知
	AddFriendRequest,           // 请求加好友

	SOCAIL_ITEM_MAX = 300,		// 社会关系数量最大值
};


// 玩家社会关系结构
struct stSocialItem
{
	U32					playerId;			// 对方玩家ID
	U8					type;				// SocialType
	U32					friendValue;		// 好友度
	stSocialItem() : playerId(0), type(0), friendValue(0)	{}

	template<class T>
	void WriteData(T* stream)
	{
		stream->writeBits(sizeof(stSocialItem)<<3,this);
	}

	template<class T>
	void ReadData(T* stream)
	{
		stream->readBits(sizeof(stSocialItem)<<3,this);
	}
};

// 玩家社会关系结构
struct stSocialInfo
{
	U32		id;					// 玩家Id
	char	name[COMMON_STRING_LENGTH];			// 名称
	U8		sex;				// 性别
	U8		level;				// 等级
	U8		family;				// 门宗
	U8      job;                // 职业
	U8		faceImage;			// 头像
	U8		status;				// 状态
	S32     teamId;             // 队伍Id
	U8		type;				// 关系类型	
	U32		friendValue;		// 好友度

	stSocialInfo()
	{
		memset( this, 0, sizeof( stSocialInfo ) );
	}

	stSocialInfo( const stSocialInfo& other )
	{
		copy( other );
	}

	stSocialInfo& operator = ( const stSocialInfo& other )
	{
		copy( other );
		return *this;
	}

	void copy( const stSocialInfo& other )
	{
		memcpy(this, &other, sizeof(stSocialInfo));
	}

	template<class T>
	void WriteData(T* stream)
	{
		stream->writeBits(sizeof(stSocialInfo)<<3,this);
	}

	template<class T>
	void ReadData(T* stream)
	{
		stream->readBits(sizeof(stSocialInfo)<<3,this);
	}
};

//出师徒弟结构
struct stLearnPrentice
{
	U32  playerId;                        // 玩家Id
	char playerName[MAX_NAME_LENGTH];     // 玩家名字
	U8   level;                           // 等级
	U8   family;                          // 门宗
	U8   job;                             // 职业
	U8   status;                          // 状态

	stLearnPrentice() {memset(this,0,sizeof(stLearnPrentice));}

	void copy(const stLearnPrentice &other)
	{
		memcpy(this, &other, sizeof(stLearnPrentice));
	}

	stLearnPrentice(const stLearnPrentice &other)
	{
		if(this != &other)
			copy(other);
	}

	stLearnPrentice& operator=(const stLearnPrentice &other)
	{
		if(this != &other)
			copy(other);

		return *this;
	}

	template<class T>
	void WriteData(T* stream)
	{
		stream->writeBits(sizeof(stLearnPrentice)<<3,this);
	}

	template<class T>
	void ReadData(T* stream)
	{
		stream->readBits(sizeof(stLearnPrentice)<<3,this);
	}

};

typedef std::pair<int,int> PlayerIdPair;
typedef std::map<PlayerIdPair, int> LimitMap;
typedef std::map<int,std::string>  SocialNameMap;
typedef std::map<int,stSocialItem> SocialItemMap;

inline bool isFriendType(U8 iType)
{
	if(iType != SocialType::Temp && iType != SocialType::Mute && iType != SocialType::Enemy)
		return true;
	else
		return false;
}

inline bool isBestFriendType(U8 iType)
{
	if(iType != SocialType::Friend && iType != SocialType::Temp && iType != SocialType::Mute && iType != SocialType::Enemy)
		return true;
	else
		return false;
}

inline bool hasMakeLink(U8 oldType,U8 newType)
{
	if(oldType == newType)
		return false;

	if (oldType != SocialType::Temp && newType == SocialType::Temp)
		return false;

	if (isBestFriendType(oldType) || isBestFriendType(newType)) 
		return false;

	if ((oldType == SocialType::Friend || oldType == SocialType::Mute) && newType == SocialType::Enemy)
		return false;

	return true;
}

struct ZoneClientSocial
{
	U32		masterID;				
	char	masterName[COMMON_STRING_LENGTH];
	U8		selfMasterLevel;

	ZoneClientSocial()
	{
		memset(this, 0, sizeof(ZoneClientSocial));
	}

	ZoneClientSocial(const ZoneClientSocial& other)
	{
		*this = other;
	}

	ZoneClientSocial& operator= (const ZoneClientSocial& other)
	{
		memcpy(this, &other, sizeof(ZoneClientSocial));
		return *this;
	}

	template<class T>
	void WriteData(T* stream)
	{
		stream->writeInt(masterID, Base::Bit32);
		stream->writeString(masterName, sizeof(masterName));
		stream->writeInt(selfMasterLevel, Base::Bit8);
	}

	template<class T>
	void ReadData(T* stream)
	{
		masterID = stream->readInt(Base::Bit32);
		stream->readString(masterName, sizeof(masterName));
		selfMasterLevel = stream->readInt(Base::Bit8);
	}
};

#endif