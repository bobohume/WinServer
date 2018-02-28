ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_IsEquip] DEFAULT ((0)) FOR [IsEquip];

