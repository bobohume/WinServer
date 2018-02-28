//-----------------------------------------------------------------------------
// PowerEngine
// Copyright (C) FireRain.com, Inc.
//-----------------------------------------------------------------------------
#include "Common/OrgDepotData.h"

#ifdef WORLDSERVER
#define GAME_ORGDEPOT_FILE "data/OrgDepotData.dat"
#define GAME_ORGBOSS_FILE "data/OrgBossData.dat"
#else
#define GAME_ORGDEPOT_FILE "gameres/data/repository/OrgDepotData.dat"
#define GAME_ORGBOSS_FILE "gameres/data/repository/OrgBossData.dat"
#endif

COrgDepot* COrgDepot::GetInstance()
{
	static COrgDepot s_orgDepot;
	return &s_orgDepot;
}

COrgDepot::COrgDepot()
:m_dataInstance(GAME_ORGDEPOT_FILE)
{
	Data data = m_dataInstance.m_dataManager.Begin();
	while (data)
	{
		m_categorys[data->GetData(Column_Category)].push_back(data);
		data = m_dataInstance.m_dataManager.Next();
	}
}

COrgDepot::Data COrgDepot::GetData(U32 itemID) const
{
	return m_dataInstance.m_dataManager.GetData(itemID);
}

const COrgDepot::Depots& COrgDepot::GetCategory(U32 category, U32 nClass) const
{
	CategorysIter iter = m_categorys.find(category);
	if (iter == m_categorys.end())
		return m_null;
	if (!nClass)
		return iter->second;
	m_class.clear();
	const Depots& depots = iter->second;
	for (DepotsIter iter=depots.begin(); iter!=depots.end(); ++iter)
	{
		const COrgDepot::Data& data = *iter;
		U32 fitClass = data->GetData(COrgDepot::Column_FitClass);
		if (fitClass && fitClass!=nClass)
			continue;
		m_class.push_back(data);
	}
	return m_class;
}

COrgDepot::Data COrgDepot::Begin() const
{
	return m_dataInstance.m_dataManager.Begin();
}

COrgDepot::Data COrgDepot::Next() const
{
	return m_dataInstance.m_dataManager.Next();
}

U32 COrgDepot::Size() const
{
	return m_dataInstance.m_dataManager.Size();
}

void COrgDepot::Reload()
{
    m_dataInstance.m_dataManager.Init(GAME_ORGDEPOT_FILE);
}

COrgBoss* COrgBoss::GetInstance()
{
	static COrgBoss s_orgBoss;
	return &s_orgBoss;
}

COrgBoss::COrgBoss()
:m_dataInstance(GAME_ORGBOSS_FILE)
{
}

COrgBoss::Data COrgBoss::GetData(U32 bossID) const
{
	return m_dataInstance.m_dataManager.GetData(bossID);
}

COrgBoss::Data COrgBoss::Begin() const
{
	return m_dataInstance.m_dataManager.Begin();
}

COrgBoss::Data COrgBoss::Next() const
{
	return m_dataInstance.m_dataManager.Next();
}

void COrgBoss::Reload()
{
    m_dataInstance.m_dataManager.Init(GAME_ORGBOSS_FILE);
}