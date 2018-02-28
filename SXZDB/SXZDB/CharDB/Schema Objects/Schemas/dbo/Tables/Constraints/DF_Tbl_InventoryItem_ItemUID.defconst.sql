ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_ItemUID] DEFAULT ((0)) FOR [ItemUID];

