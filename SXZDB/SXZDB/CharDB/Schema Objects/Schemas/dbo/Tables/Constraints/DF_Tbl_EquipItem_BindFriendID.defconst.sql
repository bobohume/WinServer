ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_BindFriendID] DEFAULT ((0)) FOR [BindFriendID];

