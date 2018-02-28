ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_IsEquip] DEFAULT ((0)) FOR [IsEquip];

