ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_EquipStrengthens] DEFAULT ((0)) FOR [EquipStrengthens];

