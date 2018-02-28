ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_Quality] DEFAULT ((0)) FOR [Quality];

