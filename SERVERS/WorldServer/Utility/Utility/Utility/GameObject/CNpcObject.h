#ifndef __HelloLua__CNpcObject__
#define __HelloLua__CNpcObject__

#include "CGameObject.h"
#include "../../Data/NpcData.h"
#include <set>

class Player;
class CNpcObject:public CGameObject
{
public:
	enum
	{
		NPC_LEVELADD_BEGIN_BUFFID   =  4510001,
		NPC_LEVELADD_END_BUFFID     =  4510200,
	};
	typedef CGameObject Parent;
	const static U32 NEXT_MASK = Parent::NEXT_MASK << 1;
	CNpcObject();
	virtual ~CNpcObject();
	virtual U32 getObjectType() { return  GAME_MONSTER; };
	virtual void Attack(CGameObject* obj);
	virtual void Spell(CGameObject* obj, U32 SkillId);
	virtual void timeSignal(float dt);

	virtual void death();
	virtual void addLevel(S32 level);
	virtual const char* getName();

	S32  randomMaster();//Ëæ»ú¹ÖÎï
	CNpcData* mDataBlock;
private:
	S32 mDataBlockId;
	U32 mActiveSkillCd;
};

#endif /* defined(__HelloLua__CNpcObject__) */
