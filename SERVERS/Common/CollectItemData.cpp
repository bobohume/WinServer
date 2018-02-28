//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FireRain.com, Inc.
//-----------------------------------------------------------------------------
#ifdef WORLDSERVER
#include "base/types.h"
#define GAME_COLLECTITEM_FILE "data/CollectItemRepository.dat"
#else
#include "platform/platform.h"
#define GAME_COLLECTITEM_FILE "gameres/data/repository/CollectItemRepository.dat"
#endif

#include "DatBuilder/DatBuilder.h"
#include "Common/DataFile.h"
#include "Common/CollectItemData.h"

#ifndef WORLDSERVER
#include "console/console.h"
#endif

CollectItemData::CollectItemData()
{
	mEventId = 0;
	mCloseInterval = 0;
}

bool CollectItemRepository::read()
{
    mCollectItemMap.clear();

	CDataFile file;
	RData lineData;
	char fileName[1024];
	file.ReadDataInit();

#ifdef WORLDSERVER
	sprintf_s( fileName,sizeof(fileName), "%s", GAME_COLLECTITEM_FILE );
#else
	Platform::makeFullPathName(GAME_COLLECTITEM_FILE,fileName,sizeof(fileName));
#endif

	if(!file.readDataFile(fileName))
	{
		file.ReadDataClose();
		SERVER_ASSERT(false,"Cannt read file : CollectItemRepository.dat");
		return false;
	}

	char buff[256] = {0,};
	for (int i=0; i<file.RecordNum; i++)
	{
		stCollectItem collectItem;
		CollectItemData* pData = new CollectItemData;

		file.GetData(lineData);
		SERVER_ASSERT(lineData.m_Type == DType_U16,"CollectItemData read error, field name 事件id");
		pData->mEventId = lineData.m_U16;

		for(int j=0; j < MAXITEM_NUM; j++)
		{
			file.GetData(lineData);
			sprintf_s(buff,sizeof(buff),"CollectItemData read error, field name 物品ID_%d",j);
			SERVER_ASSERT(lineData.m_Type == DType_U32,buff);
			collectItem.itemId = lineData.m_U32;

			file.GetData(lineData);
			sprintf_s(buff,sizeof(buff),"CollectItemData read error, field name 物品数量_%d",j);
			SERVER_ASSERT(lineData.m_Type == DType_U32,buff);
			collectItem.quantity = lineData.m_U32;

			if(collectItem.itemId != 0 && collectItem.quantity != 0)
				pData->mCollectItem.push_back(collectItem);
		}

		file.GetData(lineData);
		sprintf_s(buff,sizeof(buff),"CollectItemData read error, field name 关闭间隔时间");
		SERVER_ASSERT(lineData.m_Type == DType_F32,buff);
		pData->mCloseInterval = lineData.m_F32;

		if(!mCollectItemMap.insert(CollectItemMap::value_type(pData->mEventId,pData)).second)
		{
			delete pData;
			pData = NULL;
		}
	}

	file.ReadDataClose();
	return true;
}

CollectItemData* CollectItemRepository::getData( U16 iEventId )
{
	CollectItemMap::iterator it = mCollectItemMap.find(iEventId);
	if( it == mCollectItemMap.end() )
		return NULL;

	return it->second;
}

CollectItemRepository& CollectItemRepository::Instance()
{
	static CollectItemRepository g_Instance;
	return g_Instance;
}

void CollectItemRepository::clear()
{
	CollectItemMap::iterator it = mCollectItemMap.begin();
	for( ; it != mCollectItemMap.end(); ++it)
	{
		CollectItemData* pData = it->second;
		if( NULL == pData )
			continue;

		delete pData;
		pData = NULL;
	}

	mCollectItemMap.clear();
}

#ifndef WORLDSERVER
ConsoleFunction(getCollectItemData,const char* ,3,3,"getCollectItemData(%EventIdID,%itemCol)")
{
	S32 iEventId = dAtoi(argv[1]);
	S32 iItemCol = dAtoi(argv[2]);

	//越界了
	if(iItemCol <0 || iItemCol >= CollectItemRepository::MAXITEM_NUM )
		return "";

	CollectItemData* pData = g_CollectItemRepository.getData(iEventId);
	if(pData && pData->getSize() > iItemCol)
	{
		std::string szStr;
		stCollectItem& stItem = pData->getCollectItem(iItemCol);
		szStr += Con::getIntArg(stItem.itemId);
		szStr += " ";
		szStr += Con::getIntArg(stItem.quantity);

		char* value = Con::getReturnBuffer(512);
		dStrcpy(value, 512, szStr.c_str());
		return value;
	}

	return "";
}

#endif
