ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_LockedLeftTime] DEFAULT ((0)) FOR [LockedLeftTime];

