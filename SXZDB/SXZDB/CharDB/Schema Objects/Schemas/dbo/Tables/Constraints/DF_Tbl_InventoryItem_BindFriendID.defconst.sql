ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_BindFriendID] DEFAULT ((0)) FOR [BindFriendID];

