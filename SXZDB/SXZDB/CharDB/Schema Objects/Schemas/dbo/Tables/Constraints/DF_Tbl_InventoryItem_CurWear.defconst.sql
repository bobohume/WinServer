ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_CurWear] DEFAULT ((0)) FOR [CurWear];

