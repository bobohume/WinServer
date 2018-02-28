#include "DropManager.h"
#include "Common/mRandom.h"
#include "../../AccountHandler.h"
#include "../../WorldServer.h"
#include "../../PlayerMgr.h"
#include "../../data/NpcData.h"
#include "../../data/ItemData.h"
#include "../../data/DropData.h"
#include "../../DB_SavePlayerItem.h"
#include "../../db/Drop/DB_DelDrop.h"
#include "../../db/Drop/DB_SaveDrop.h"
#include "Common/UUId.h"
#include "Common/Define.h"

bool DropManager::saveToDB(U32 nPlayerId, stDropInfo* pDrop)
{
	DB_SaveDrop* pDBHandle = new DB_SaveDrop();
	pDBHandle->mPlayerId = nPlayerId;
	pDBHandle->pDrop = pDrop;
	SERVER->GetMailDBManager()->SendPacket(pDBHandle);
	return true;
}

bool DropManager::delToDB(std::string UID)
{
	DB_DelDrop* pDBHandle = new DB_DelDrop();
	pDBHandle->mUID = UID;
	SERVER->GetMailDBManager()->SendPacket(pDBHandle);
	return true;
}

bool DropManager::delToDB(U32 nPlayerID)
{
	DB_DelDrop* pDBHandle = new DB_DelDrop();
	pDBHandle->mPlayerId = nPlayerID;
	pDBHandle->mbDelPlayer = true;
	SERVER->GetMailDBManager()->SendPacket(pDBHandle);
	return true;
}

bool DropManager::loadDB()
{
	DBError err = DBERR_UNKNOWERR;
	CDBConnPool* pDBPool = SERVER->GetActorDB()->GetPool();
	if(pDBPool)
	{
		CDBConn* pDBConn = pDBPool->Lock();
		if(pDBConn)
		{
			do 
			{
				try
				{
					M_SQL(pDBConn, "SELECT * FROM Tbl_Drop");
					while(pDBConn->More())
					{
						U32 nPlayerId = 0;
						stDropInfo* stDrop = new stDropInfo();
						stDrop->UID			= pDBConn->GetString();
						nPlayerId			= pDBConn->GetInt();
						stDrop->nItemId		= pDBConn->GetInt();
						DROPMGR->insertDropMap(nPlayerId, stDrop, false);
					}

					err = DBERR_NONE;
				}
				DBECATCH()
			} while (false);
			pDBPool->Unlock(pDBConn);
		}
	}

	if(err == DBERR_NONE)
		return true;

	return false;
}

//掉落处理，可以是包掉包
CDropData* DropManager::dropItemEx(U32 dropId)
{
	U32 nMaxRandomVal = 0;
	CDropDataRes::CROPDATAVEC DropVec;
	CDropData* pData = NULL;
	g_DropDataMgr->getDropData(dropId, DropVec, nMaxRandomVal);
	if(DropVec.empty())
	{
		g_Log.WriteWarn("DropManager dropItem, Player [%d] dropid [%d] is not found in data", dropId);
		return NULL;
	}

	S32 nRandomVal = gRandGen.randI(0, nMaxRandomVal);
	for(CDropDataRes::CROPDATAITR itr = DropVec.begin(); itr != DropVec.end(); ++itr)
	{
		if(*itr)
		{
			nRandomVal -= (*itr)->m_Odds * 1000;
			if(nRandomVal <= 0)
			{
				pData = *itr;
				break;
			}
		}
	}

	//最后一个掉落物品
	if(nRandomVal > 0)
	{
		pData = *(DropVec.end());	
	}
	return pData;
}

//掉落物品处理
bool DropManager::doDropItem(U32 dropId, U32 nPlayerId, U32& nItemId, std::string& UID)
{
	CDropData* pData = dropItemEx(dropId);
	if(!pData)
		return false;

	//包掉包
	while(pData->m_Type == DropBag)
	{
		pData = dropItemEx(pData->m_Odds);
		if(!pData)
			return false;
	}

	if(pData)
	{
		nItemId =  pData->m_GoodsId;
		CItemData* pItemData = g_ItemDataMgr->getItemData(nItemId);
		if(!pItemData)
			return false;

		UID = uuid();
		stDropInfo* stDrop = new stDropInfo();
		stDrop->nItemId	 = nItemId;
		stDrop->UID = UID;

		insertDropMap(nPlayerId, stDrop);
		return true;
	}

	return false;
}

bool DropManager::dropMapManCreate(U32 nItemId, U32 nPlayerId, std::string& UID)
{
	CItemData* pItemData = g_ItemDataMgr->getItemData(nItemId);
	if(!pItemData)
		return false;

	UID = uuid();
	stDropInfo* stDrop = new stDropInfo();
	stDrop->nItemId	 = nItemId;
	stDrop->UID = UID;

	insertDropMap(nPlayerId, stDrop);
	return true;
}

U32 DropManager::dropItem(U32 nNpcId)
{
	CNpcData* pNpcData	 = g_NpcDataMgr->getNpcData(nNpcId);
	if(!pNpcData || !pNpcData->m_Drop)
		return 0;

	U32 nRandomVal = gRandGen.randI(1, 1000);
	//掉落物品
	if(nRandomVal <= (pNpcData->m_DropVal * 1000))
	{
		return pNpcData->m_Drop;
	}

	return 0;
}

stDropInfo* DropManager::getDropInfo(U32 nPlayerId, std::string UID)
{
	PLAYERDROPITR itr = m_PlayerDropMap.find(nPlayerId);
	if(itr != m_PlayerDropMap.end())
	{
		DROPITR itr = m_PlayerDropMap[nPlayerId].find(UID);
		if(itr != m_PlayerDropMap[nPlayerId].end())
		{
			if(itr->second)
				return itr->second;
		}
	}

	return NULL;
}

void DropManager::delDropInfo(U32 nPlayerId, std::string UID)
{
	PLAYERDROPITR itr = m_PlayerDropMap.find(nPlayerId);
	if(itr != m_PlayerDropMap.end())
	{
		DROPITR iter = m_PlayerDropMap[nPlayerId].find(UID);
		if(iter != m_PlayerDropMap[nPlayerId].end())
		{
			SAFE_DELETE(iter->second);
		}
		m_PlayerDropMap[nPlayerId].erase(UID);
		delToDB(UID);
	}
}

void DropManager::clearDropMap(U32 nPlayerId)
{
	PLAYERDROPITR itr = m_PlayerDropMap.find(nPlayerId);
	if(itr != m_PlayerDropMap.end())
	{
		for(DROPITR iter = itr->second.begin(); iter != itr->second.end(); ++iter)
		{
			SAFE_DELETE(iter->second);
		}

		itr->second.clear();

		m_PlayerDropMap.erase(nPlayerId);
		delToDB(nPlayerId);
	}
}

void DropManager::insertDropMap(U32 nPlayerId, stDropInfo* dropInfo, bool bSaveToDB/*=true*/)
{
	if(!dropInfo)
		return;

	PLAYERDROPITR itr = m_PlayerDropMap.find(nPlayerId);
	if(itr == m_PlayerDropMap.end())
	{
		DROPMAP dropMap;
		m_PlayerDropMap.insert(PLAYERDROPMAP::value_type(nPlayerId, dropMap));
	}

	m_PlayerDropMap[nPlayerId].insert(DROPMAP::value_type(dropInfo->UID, dropInfo));
	if(bSaveToDB)
	{
		saveToDB(nPlayerId, dropInfo);
	}
}