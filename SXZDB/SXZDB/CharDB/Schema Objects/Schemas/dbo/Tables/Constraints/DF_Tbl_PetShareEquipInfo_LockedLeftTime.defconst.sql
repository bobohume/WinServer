ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_LockedLeftTime] DEFAULT ((0)) FOR [LockedLeftTime];

