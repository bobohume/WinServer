ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_CurMaxWear] DEFAULT ((0)) FOR [CurMaxWear];

