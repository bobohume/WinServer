ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_DBVersion] DEFAULT ((0)) FOR [DBVersion];

