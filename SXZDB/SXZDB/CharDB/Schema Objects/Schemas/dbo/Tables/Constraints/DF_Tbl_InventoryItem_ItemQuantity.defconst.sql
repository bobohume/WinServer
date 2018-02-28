ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_ItemQuantity] DEFAULT ((0)) FOR [Quantity];

