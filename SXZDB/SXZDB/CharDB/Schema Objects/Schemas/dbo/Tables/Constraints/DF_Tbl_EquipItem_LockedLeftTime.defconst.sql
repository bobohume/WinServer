ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_LockedLeftTime] DEFAULT ((0)) FOR [LockedLeftTime];

