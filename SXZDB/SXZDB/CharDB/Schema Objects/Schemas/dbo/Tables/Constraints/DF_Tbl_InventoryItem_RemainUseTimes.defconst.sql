ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_RemainUseTimes] DEFAULT ((0)) FOR [RemainUseTimes];

