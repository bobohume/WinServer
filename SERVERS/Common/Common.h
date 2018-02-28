#ifndef COMMON_H
#define COMMON_H

#include "PlayerStruct.h"
#include "PacketType.h"
#include <vector>
#include <string>

enum enActivityType
{
	ACTIVITY_CHARGE_6 = 0,
	ACTIVITY_CHARGE_30,
	ACTIVITY_CHARGE_68,
	ACTIVITY_CHARGE_198,
	ACTIVITY_CHARGE_328,
	ACTIVITY_CHARGE_648,
	ACTIVITY_TANGUAN,
	ACTIVITY_BATTLE,
	ACTIVITY_CARD,
	ACTIVITY_PRINCESS,
	ACTIVITY_LAND,
	ACTIVITY_CHARGE_8,
	ACTIVITY_CHARGE_12,
	ACTIVITY_CHARGE_88,
	ACTIVITY_CHARGE_128,
	ACTIVITY_GOLD,
	ACTIVITY_MONEY,
	ACTIVITY_ORE,
	ACTIVITY_TROOP,
	ACTIVITY_LOGIN,
	ACTIVITY_DINNER,
	ACTIVITY_IMPOSE,
	ACTIVITY_READAPPLY,
	ACTIVITY_ITEM1,
	ACTIVITY_ITEM2,
	ACTIVITY_ITEM3,
	ACTIVITY_ITEM4,
	ACTIVITY_PLANT,
	ACTIVITY_TURNCARD,
	ACTIVITY_LEARNRITE,
	ACTIVITY_VISITYCITY,
	ACTIVITY_DISPATCH,
	ACTIVITY_PVP,
	ACTIVITY_PRINCE,

	ACTIVITY_MAX,
};

enum
{
	ACTIVITY_NOTIFY_GOLD = 0,
};
class CConfig
{
public:
	static CConfig* GetInstance();
	int GetGameType();
	void SetGameType(int type);

private:
	CConfig();

	int m_gametype;
};

#define CONFIG CConfig::GetInstance()

#define MAX_QUERY_PRESENTEE	600		//一个推广员最多有600个下线帐号

#define ACCOUNTFLAG_MAX	5		//帐号特殊标志数组的长度
#define UPDATE_TIME_INTERVAL 3
enFamily GetFamilyByZone(int zoneID);
int GetZoneByFamily(enFamily family);
int GetBirthZoneByFamily(enFamily family);

#ifdef NTJ_SERVER
U32 getCreateFace(U32 sex, U32 face);
U32 getCreateHair(U32 sex, U32 hair, U32 hairColor);
#endif

#ifdef NTJ_CLIENT
U32 getCreateFace(U32 sex, U32 face);
U32 getCreateHair(U32 sex, U32 hair, U32 hairColor);
#endif

struct CreatePlayerInfo
{
	char	m_CreateName[COMMON_STRING_LENGTH];
	T_UID	m_CreateUID;
	int		m_CreateBody;
	int		m_CreateSex;
	int		m_CreateFace;
	int		m_CreateHair;
	int		m_CreateHairColor;
	int		m_CreateItemId;
	int		m_CreateFaceImage;
	int		m_Family;

	CreatePlayerInfo()
	{
		Clear();
	}

	CreatePlayerInfo(const CreatePlayerInfo& other)
	{
		*this = other;
	}

	void Clear()
	{
		memset(this, 0, sizeof(CreatePlayerInfo));
	}

	CreatePlayerInfo& operator=(const CreatePlayerInfo& other)
	{
		memcpy(this, &other, sizeof(CreatePlayerInfo));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(m_CreateUID, UID_BITS);
		packet->writeString(m_CreateName, sizeof(m_CreateName));
		packet->writeInt(m_CreateSex, Base::Bit8);
		packet->writeInt(m_CreateBody, Base::Bit16);
		packet->writeInt(m_CreateFace, Base::Bit32);
		packet->writeInt(m_CreateHair, Base::Bit16);
		packet->writeInt(m_CreateHairColor, Base::Bit8);
		packet->writeInt(m_CreateItemId, Base::Bit32);
		packet->writeInt(m_CreateFaceImage, Base::Bit8);
		packet->writeInt(m_Family, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		m_CreateUID			=	packet->readInt(UID_BITS);
		packet->readString(m_CreateName, sizeof(m_CreateName));
		m_CreateSex			=	packet->readInt(Base::Bit8);
		m_CreateBody		=	packet->readInt(Base::Bit16);
		m_CreateFace		=	packet->readInt(Base::Bit32);
		m_CreateHair		=	packet->readInt(Base::Bit16);
		m_CreateHairColor	=	packet->readInt(Base::Bit8);
		m_CreateItemId		=	packet->readInt(Base::Bit32);
		m_CreateFaceImage	=	packet->readInt(Base::Bit8);
		m_Family			=	packet->readInt(Base::Bit32);
	}
};

#define MATRIX_CARD_POSITIONS 3
#define MATRIX_CARD_WIDTH 9
#define MSTRIX_CARD_HEIGTH 9
#define MSTRIX_SEED_LENGTH 32

struct MatrixPos
{
	int pos[MATRIX_CARD_POSITIONS][2];

	MatrixPos()
	{
		Clear();
	}

	MatrixPos(const MatrixPos& other)
	{
		*this = other;
	}

	void Clear()
	{
		memset(this, 0, sizeof(MatrixPos));
	}
	
	bool HasSamePos()
	{
		for (int i=0; i<MATRIX_CARD_POSITIONS; ++i)
		{
			for (int j=i+1; j<MATRIX_CARD_POSITIONS; ++j)
			{
				if (pos[i][0]==pos[j][0] && pos[i][1]==pos[j][1])
					return true;
			}
		}
		return false;
	}

	MatrixPos& operator=(const MatrixPos& other)
	{
		memcpy(this, &other, sizeof(MatrixPos));
		return *this;
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		for (int i=0; i<MATRIX_CARD_POSITIONS; ++i)
		{
			packet->writeInt(pos[i][0], Base::Bit8);
			packet->writeInt(pos[i][1], Base::Bit8);
		}
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		for (int i=0; i<MATRIX_CARD_POSITIONS; ++i)
		{
			pos[i][0] = packet->readInt(Base::Bit8);
			pos[i][1] = packet->readInt(Base::Bit8);
		}
	}
};

enum LOGIN_TYPE
{
	LOGIN_Normal,
	LOGIN_TWeibo,
	LOGIN_YY,
	LOGIN_Other,
	LOGIN_TYPE_Count
};

const char* LOGIN_TYPE_STR[];

LOGIN_TYPE GetLoginType(const char* loginTypeStr);
const char* GetLoginTypeStr(LOGIN_TYPE loginType);
bool IsTelePhone(const char* telePhone);

enum SPONSOR_TYPE
{
	SPONSOR_WEIXING,
	SPONSOR_WANGWANG,
};

#define RMB_GAME_RATIO 100

struct AlipayInfo
{
	char accountName[ACCOUNT_NAME_LENGTH];
	int accountID;
	int playerID;
	char IP[COMMON_STRING_LENGTH];
	int orderID;
	int price;
	int areaId;
	int socket;

	AlipayInfo()
	{
		Clear();
	}

	AlipayInfo(const AlipayInfo& other)
	{
		*this = other;
	}

	void Clear()
	{
		memset(this, 0, sizeof(AlipayInfo));
	}

	AlipayInfo& operator=(const AlipayInfo& other)
	{
		memcpy(this, &other, sizeof(AlipayInfo));
		return *this;
	}
};

struct PlayerInfo
{
	PlayerInfo()
	{
		Clear();
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeBits(sizeof(PlayerInfo)<<3, this);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		packet->readBits(sizeof(PlayerInfo)<<3, this);
	}

	void Clear()
	{
		memset(this, 0, sizeof(PlayerInfo));
	}

	int playerID;
	bool rename;
};

struct stOrder
{
public:
	stOrder() :id(0), buytype(0), itemid(0), itemcount(0){
	}

	template<typename _Ty>
	void WriteData(_Ty* packet) const
	{
		packet->writeInt(id, Base::Bit32);
		packet->writeInt(buytype, Base::Bit32);
		packet->writeInt(itemid, Base::Bit32);
		packet->writeInt(itemcount, Base::Bit32);
		packet->writeInt(price, Base::Bit32);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		id = packet->readInt(Base::Bit32);
		buytype = packet->readInt(Base::Bit32);
		itemid = packet->readInt(Base::Bit32);
		itemcount = packet->readInt(Base::Bit32);
		price = packet->readInt(Base::Bit32);
	}

	int id;
	int buytype;
	int itemid;
	int itemcount;
	int price;
};

typedef std::vector<PlayerInfo> Players;
typedef Players::iterator PlayersIter;

#endif /*COMMON_H*/