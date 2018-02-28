ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_SkillBForEquip] DEFAULT ((0)) FOR [SkillBForEquip];

