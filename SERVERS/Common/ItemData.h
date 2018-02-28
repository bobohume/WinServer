#ifndef	_ITEMDATA_H
#define _ITEMDATA_H
/*
#include "Common/OrgDepotData.h"

class CItemData
{
public:
	enum Column
	{
		Column_ItemID, //道具编号
		Column_ItemName, //道具名称
		Column_Category, //道具类别
		Column_SubCategory, //道具子类
		Column_DescriptionID, //道具描述
		Column_HelpID, //道具说明
		Column_PurposeID, //用途
		Column_IconName, //道具icon
		Column_PetInsight, //宠物悟性
		Column_PetTalent, //宠物根骨
		Column_SoundIDA, //攻击音效A
		Column_EffectSoundID, //重击音效
		Column_TimeMode, //时效计算方式
		Column_Duration, //时效
		Column_SaleType, //出售方式
		Column_SalePrice, //出售价格
		Column_MaxOverNum, //最大叠加数
		Column_Reserve, //备用字段
		Column_MissionID, //任务编号
		Column_MissionNeed, //任务提示
		Column_ColorLevel, //颜色等级
		Column_QualityLevel, //品质等级
		Column_DropRate, //掉落几率
		Column_BaseLimit, //基本限制
		Column_SexLimit, //性别限制
		Column_LevelLimit, //等级限制
		Column_FamilyLimit, //门宗限制
		Column_UseTimes, //使用次数
		Column_Renascences, //转生次数
		Column_CoolTimeType, //物品冷却类型
		Column_CoolTime, //冷却时间
		Column_VocalTime, //吟唱时间
		Column_IdentifyType, //鉴定
		Column_RandomNameID, //名称随机属性ID
		Column_SuitID, //套装编号
		Column_BaseAttribute, //装备主属性

		Column_AppendAttribute_1,
		Column_AppendAttribute_12 = Column_AppendAttribute_1+11,

		Column_SkillAForEquip, //主动技能
		Column_SkillBForEquip, //被动技能
		Column_SkillForUse, //使用时技能
		Column_MaxWear, //最大耐久
		Column_MaxEnhanceWears, //最大强化次数
		Column_BindMode, //物品绑定模式
		Column_BindPro, //灵魂绑定属性
		Column_AllowEmbedSlot, //允许镶嵌孔
		Column_OpenedEmbedSlot, //已开启镶嵌孔

		Column_MaleEquipEffectID, //男装备特效ID
		Column_MaleEquipStrengthenEffectID, //男强化特效ID
		Column_FemaleEquipEffectID,
		Column_FemaleEquipStrengthenEffectID,
		Column_EquipStrengthenEffectLevel, //特效等级限制

		Column_EquipStrengthen_1, //1级装备属性
		Column_EquipStrengthenSkin_1, //1级贴图
		Column_EquipStrengthen_15 = Column_EquipStrengthen_1+2*14,
		Column_EquipStrengthenSkin_15 = Column_EquipStrengthenSkin_1+2*14,

		Column_EquipStrengthenLevel_1,
		Column_EquipStrengthenLevel1_1,
		Column_EquipStrengthenLevel_3 = Column_EquipStrengthenLevel_1+2*2,
		Column_EquipStrengthenLevel1_3 = Column_EquipStrengthenLevel1_1+2*2,

		Column_MaleShapesSetId, //模型组编号(男)
		Column_FemaleShapesSetId, //模型组编号(女)
		Column_NewFamilyMaleShapesSetId, //男模型组编号(剑灵)
		Column_NewFamilyFemaleShapesSetId,
		Column_StoneLimit,   //宝石鉴定
		Column_TrailEffect,  //拖尾特效
		Column_SoundIDB,     //攻击音效B
		Column_PetType,      //灵兽类型
		Column_ExFlags,      //额外功能标志
		Column_JobLimit,     //职业限制
		Column_Priority,     //排序优先级
		Column_ProductType,  //产出方式
		Column_ProductWarns, //产出数量报警
		Column_SoulStoneLimit, //魂装宝石限制
		Column_SkillSlotType,  //技能插槽类型

		Column_Count
	};

	typedef TDataInstance<CItemData>::DataManager::DataPtr Data;

	static CItemData* GetInstance();
	Data GetData(U32 itemID) const;
	Data Begin() const;
	Data Next() const;
    
    void Reload(void);
private:
	CItemData();

	TDataInstance<CItemData> m_dataInstance;
};

#define ITEMDATA CItemData::GetInstance()
*/
#endif //_ITEMDATA_H