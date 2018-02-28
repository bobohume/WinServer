/*
#include "CollectMgr.h"
#include "DBLayer/Data/TBLCollectEvent.h"
#include "Common/CollectItemData.h"
#include "WorldServer.h"
#include "ServerMgr.h"
#include "DB_SaveCollect.h"
#include "wintcp/dtServerSocket.h"

bool CCollectManager::isOpen(int iEventId)
{
	DO_LOCK(Lock::Player);

	CollectsIter it = m_collectEvents.find(iEventId);
	if(it != m_collectEvents.end())
		return true;

	return false;
}

bool CCollectManager::isZoneId(int zoneId)
{
	if(zoneId >9999)
		return false;
	else
		return true;
}

bool CCollectManager::isClosed(const stCollectEvent& collect)
{
	DO_LOCK(Lock::Player);

	 if(collect.bComplete)
	 {
		 g_Log.WriteLog("开服时,全服收集完成,事件Id[%d]",collect.eventid);
		 return true;
	 }

	 CollectItemData* pData = g_CollectItemRepository.getData(collect.eventid);
	 if (NULL == pData)
		 return false;

	 int time = pData->getCloseInterval();
	 if(time == 0)
		 return false;

	 __time32_t currTime = _time32(NULL);
	 if(currTime>= (collect.openTime + time))
	 {
		 g_Log.WriteLog("全服收集时间推进到期,事件Id[%d],当前时间[%d]>=开启时间[%d]+到期时间[%d]",
			 collect.eventid,currTime,collect.openTime,time);
		 return true;
	 }

	 return false;
}

bool CCollectManager::isComplete(const stCollectEvent& collect)
{
	DO_LOCK(Lock::Player);

	CollectItemData* pData = g_CollectItemRepository.getData(collect.eventid);
	if (NULL == pData)
		return false;

	U32 size = pData->getSize();
	for (U32 i=0; i<size; i++)
	{
		if (collect.itemnum[i] < pData->getCollectItem(i).quantity)
			return false;
	}
	return true;
}

CCollectManager::CCollectManager()
{
	std::vector<stCollectEvent> collectEvents;
	TBLCollectEvent tblCollect(SERVER_CLASS_NAME::GetActorDB());
	
	DBError dbError = tblCollect.Load(collectEvents);
	if(DBERR_NONE == dbError)
	{
		for each(const stCollectEvent& collect in collectEvents)
			m_collectEvents.insert(CollectEvents::value_type(collect.eventid, collect));
	}
	else
	{
		g_Log.WriteError("全服收集事件初始化失败error=%d",getDBErr(dbError));
	}

	registerEvent(COLLECT_Collect, &CCollectManager::HandleCollect);
}

void CCollectManager::SendCollectToZone(int sockId, int zoneId)
{
	DO_LOCK(Lock::Player);

	/*if (!isZoneId(zoneId))
		return;*/
	/*
	//过滤超时或已经完成的事件
	CollectsConIter iter = m_collectEvents.begin();
	while(iter != m_collectEvents.end())
	{
		const stCollectEvent& collect = iter->second;
		bool bFlag = isClosed(collect);
		if(bFlag)
		{
			iter = m_collectEvents.erase(iter);
		}
		else
		{
			++iter;
		}
	}
	
	//这里是不可能这么大的,但限制一下.
	//如查超了,说明策划脚本有问题了
	int iSize = (int)m_collectEvents.size();
	if(iSize > 200)
	{
		iSize = 200;
		g_Log.WriteError("正在进行中的全服收集事件已经超过200了，非常可怕的事情，谁干的!");
	}

	char buf[MAX_PACKET_SIZE];
	Base::BitStream sendPacket(buf,sizeof(buf));
	stPacketHead* pSendHead = IPacket::BuildPacketHead(sendPacket, COLLECT_Info, 0, SERVICE_ZONESERVER, zoneId);

	sendPacket.writeInt(iSize, Base::Bit8);
	iter = m_collectEvents.begin();

	for (int i=0; i < iSize && iter != m_collectEvents.end(); ++iter,++i)
	{
		const stCollectEvent& collect = iter->second;
		collect.Pack(&sendPacket);
	}

	SERVER->GetServerSocket()->Send(sockId,sendPacket);
}

void CCollectManager::SetCollect(stCollectEvent& collect)
{
    DO_LOCK(Lock::Player);

    m_collectEvents[collect.eventid] = collect;
}

void CCollectManager::EVENT_FUNCTION(HandleCollect)
{
	DO_LOCK(Lock::Player);

	int ZoneId = pHead->SrcZoneId;
	stCollectEvent temp;
	temp.Unpack(Packet);

	bool bFlage = isOpen(temp.eventid);
	stCollectEvent collect = m_collectEvents[temp.eventid];

	if(collect.bComplete)
	{
		g_Log.WriteLog("全服收集已经完成,事件Id[%d],zoneId[%d]",temp.eventid,ZoneId);
		return;
	}

	collect += temp;

	//是否初始化
	if(!bFlage)
	{
		collect.eventid = temp.eventid;
		collect.openTime = _time32(NULL);
	}

	//是否收集完成
	if(!collect.bComplete)
		collect.bComplete = isComplete(collect);

	//完成记录日志
	if(collect.bComplete)
		g_Log.WriteLog("全服收集完成,事件Id[%d],zoneId[%d]",collect.eventid,ZoneId);

	DB_SaveCollect* pDBHandle = new DB_SaveCollect;
    pDBHandle->m_collectEvent = collect;
    SERVER->GetCommonDBManager()->SendPacket(pDBHandle);
}
*/