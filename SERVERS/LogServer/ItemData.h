#ifndef	_ITEMDATA_H
#define _ITEMDATA_H

#include "Common/OrgDepotData.h"

class CItemData
{
public:
	enum Column
	{
		Column_ItemID,
		Column_ItemName,
		Column_Category,
		Column_SubCategory,
		Column_DescriptionID,
		Column_HelpID,
		Column_PurposeID,
		Column_IconName,
		Column_PetInsight,
		Column_PetTalent,
		Column_SoundIDA,
		Column_EffectSoundID,
		Column_TimeMode,
		Column_Duration,
		Column_SaleType,
		Column_SalePrice,
		Column_MaxOverNum,
		Column_Reserve,
		Column_MissionID,
		Column_MissionNeed,
		Column_ColorLevel,
		Column_QualityLevel,
		Column_DropRate,
		Column_BaseLimit,
		Column_SexLimit,
		Column_LevelLimit,
		Column_FamilyLimit,
		Column_UseTimes,
		Column_Renascences,
		Column_CoolTimeType,
		Column_CoolTime,
		Column_VocalTime,
		Column_IdentifyType,
		Column_RandomNameID,
		Column_SuitID,
		Column_BaseAttribute,

		Column_AppendAttribute_1,
		Column_AppendAttribute_12 = Column_AppendAttribute_1+11,

		Column_SkillAForEquip,
		Column_SkillBForEquip,
		Column_SkillForUse,
		Column_MaxWear,
		Column_MaxEnhanceWears,
		Column_BindMode,
		Column_BindPro,
		Column_AllowEmbedSlot,
		Column_OpenedEmbedSlot,

		Column_MaleEquipEffectID,
		Column_MaleEquipStrengthenEffectID,
		Column_FemaleEquipEffectID,
		Column_FemaleEquipStrengthenEffectID,
		Column_EquipStrengthenEffectLevel,

		Column_EquipStrengthen_1,
		Column_EquipStrengthenSkin_1,
		Column_EquipStrengthen_15 = Column_EquipStrengthen_1+2*14,
		Column_EquipStrengthenSkin_15 = Column_EquipStrengthenSkin_1+2*14,

		Column_EquipStrengthenLevel_1,
		Column_EquipStrengthenLevel1_1,
		Column_EquipStrengthenLevel_3 = Column_EquipStrengthenLevel_1+2*2,
		Column_EquipStrengthenLevel1_3 = Column_EquipStrengthenLevel1_1+2*2,

		Column_MaleShapesSetId,
		Column_FemaleShapesSetId,
		Column_StoneLimit,
		Column_TrailEffect,
		Column_SoundIDB,
		Column_PetType,
		Column_ExFlags,
		Column_JobLimit,
		Column_Priority,
		Column_ProductType,
		Column_ProductWarns,

		Column_Count
	};

	typedef TDataInstance<CItemData>::DataManager::DataPtr Data;

	static CItemData* GetInstance();
	Data GetData(U32 itemID) const;
	Data Begin() const;
	Data Next() const;

private:
	CItemData();

	TDataInstance<CItemData> m_dataInstance;
};

#define ITEMDATA CItemData::GetInstance()

#endif //_ITEMDATA_H