ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_LapseTime] DEFAULT ((0)) FOR [LapseTime];

