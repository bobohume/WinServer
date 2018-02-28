ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_LapseTime] DEFAULT ((0)) FOR [LapseTime];

