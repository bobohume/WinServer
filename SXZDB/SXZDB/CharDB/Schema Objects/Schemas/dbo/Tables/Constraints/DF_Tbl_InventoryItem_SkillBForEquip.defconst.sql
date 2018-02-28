ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_SkillBForEquip] DEFAULT ((0)) FOR [SkillBForEquip];

