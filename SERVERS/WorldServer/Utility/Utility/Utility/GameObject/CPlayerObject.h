#ifndef __HelloLua__CPlayerObject__
#define __HelloLua__CPlayerObject__

#include "CGameObject.h"
#include "Common/PacketType.h"
class Player:public CGameObject
{
public:
	typedef CGameObject Parent;
	const static char* ACTION_ANIMATION;
	enum
	{
		MAX_ACTION_NUM = 5,
		PLAYER_LEVELADD_BEGIN_BUFFID   =  4500001,
		PLAYER_LEVELADD_END_BUFFID     =  4500200,
	};

	const static U32 ROUTING_MASK       = Parent::NEXT_MASK << 0;//寻径标志
	const static U32 NEXT_MASK          = Parent::NEXT_MASK << 1;

	Player();
	virtual ~Player();
	virtual U32 getObjectType() { return GAME_PLAYER; };
	virtual void Attack(CGameObject* obj);
	virtual void Spell(CGameObject* obj, U32 SkillId);
	virtual void timeSignal(float dt);

	bool    isDeath() { return false; }//测试玩家不死
	inline bool    isRounting()        {return mMask & ROUTING_MASK; };
	
	void setGateId(U32 gateId)      { m_GateId = gateId; }
	U32  getGateId()                { return m_GateId; }
	inline U32  getPlayerId()   { return m_PlayerId; }
	inline void setPlayerId(U32 nId) { m_PlayerId = nId; }

	virtual const char* getName();
private:
	U32 m_GateId;//关卡id
	U32 m_PlayerId;
};

//玩家创建角色信息
struct CreatePlayerInfo
{
	char	m_CreateName[COMMON_STRING_LENGTH];
	unsigned int	m_CreateUID;
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
		packet->writeInt(m_CreateFace, Base::Bit16);
		packet->writeInt(m_CreateHair, Base::Bit16);
		packet->writeInt(m_CreateHairColor, Base::Bit8);
		packet->writeInt(m_CreateItemId, Base::Bit32);
		packet->writeInt(m_CreateFaceImage, Base::Bit8);
		packet->writeInt(m_Family, Base::Bit8);
	}

	template<typename _Ty>
	void ReadData(_Ty* packet)
	{
		m_CreateUID			=	packet->readInt(UID_BITS);
		packet->readString(m_CreateName, sizeof(m_CreateName));
		m_CreateSex			=	packet->readInt(Base::Bit8);
		m_CreateBody		=	packet->readInt(Base::Bit16);
		m_CreateFace		=	packet->readInt(Base::Bit16);
		m_CreateHair		=	packet->readInt(Base::Bit16);
		m_CreateHairColor	=	packet->readInt(Base::Bit8);
		m_CreateItemId		=	packet->readInt(Base::Bit32);
		m_CreateFaceImage	=	packet->readInt(Base::Bit8);
		m_Family			=	packet->readInt(Base::Bit8);
	}
};
#endif /* defined(__HelloLua__CPlayerObject__) */
