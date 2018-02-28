#include "ItemData.h"
/*
#define GAME_ITEMDATA_FILE "data/ItemRepository.dat"

CItemData* CItemData::GetInstance()
{
	static CItemData s_itemData;
	return &s_itemData;
}

CItemData::CItemData()
:m_dataInstance(GAME_ITEMDATA_FILE)
{
}

CItemData::Data CItemData::GetData(U32 itemID) const
{
	return m_dataInstance.m_dataManager.GetData(itemID);
}

CItemData::Data CItemData::Begin() const
{
	return m_dataInstance.m_dataManager.Begin();
}

CItemData::Data CItemData::Next() const
{
	return m_dataInstance.m_dataManager.Next();
}

void CItemData::Reload(void)
{
    m_dataInstance.m_dataManager.Init(GAME_ITEMDATA_FILE);
}*/