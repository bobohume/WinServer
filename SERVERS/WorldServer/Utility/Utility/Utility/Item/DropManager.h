#ifndef _UTILITY_ITEM_DROPMANAGER_H_
#define _UTILITY_ITEM_DROPMANAGER_H_
#include "base/types.h"
#include "Common/PlayerStruct.h"
#include "CommLib/CommLib.h"
#include <set>
#include <vector>
#include <string>
#include <hash_map>

enum en_DropType
{
	DROP_TYPE_NPC           = 1,//怪物掉落
	DROP_TYPE_AUTODROP      = 2,//场景自动掉落
	DROP_TYPE_MANDROP       = 3,//固定房间手动掉落
	DROP_TYPE_MAX,
};

//物品掉落管理
struct stDropInfo
{
	std::string UID;
	U32 nItemId;

	stDropInfo(): nItemId(0), UID("")
	{
	}
};

class CDropData;

class DropManager
{
public:
	typedef stdext::hash_map<std::string, stDropInfo*> DROPMAP;
	typedef DROPMAP::iterator DROPITR;
	typedef stdext::hash_map<U32, DROPMAP>	PLAYERDROPMAP;
	typedef PLAYERDROPMAP::iterator PLAYERDROPITR;

	DropManager()
	{

	}

	~DropManager()
	{
		for(PLAYERDROPITR itr = m_PlayerDropMap.begin(); itr != m_PlayerDropMap.end(); ++itr)
		{
			for(DROPITR iter = itr->second.begin(); iter != itr->second.end();  ++iter)
			{
				SAFE_DELETE(iter->second);
			}
			itr->second.clear();
		}
		m_PlayerDropMap.clear();
	}
	//--------------------数据库操作-------------------------
	bool        saveToDB(U32 nPlayerId, stDropInfo* pDrop);
	bool        delToDB(std::string UID);
	bool		delToDB(U32 nPlayerID);
	bool		loadDB();

	void clearDropMap(U32 nPlayerId);
	void insertDropMap(U32 nPlayerId, stDropInfo* dropInfo, bool bSaveToDB = true);

	stDropInfo* getDropInfo(U32 nPlayerId, std::string UID);
	void		delDropInfo(U32 nPlayerId, std::string UID);
	bool		doDropItem(U32 dropId, U32 nPlayerId, U32& nItemId, std::string& UID);//掉落物品
	CDropData*	dropItemEx(U32 dropId);//这里是真正掉落
	U32			dropItem(U32 nNpcId);//返回掉落包id
	bool		dropMapManCreate(U32 nItemId, U32 nPlayerId, std::string& UID);

	static DropManager* getInstance()
	{
		static DropManager s_cDropMgr;
		return &s_cDropMgr;
	}
private:
	PLAYERDROPMAP m_PlayerDropMap;
};

#define  DROPMGR DropManager::getInstance()

#endif