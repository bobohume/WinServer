ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_BindFriendID] DEFAULT ((0)) FOR [BindFriendID];

