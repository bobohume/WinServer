ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_LapseTime] DEFAULT ((0)) FOR [LapseTime];

