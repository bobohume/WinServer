ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_RemainUseTimes] DEFAULT ((0)) FOR [RemainUseTimes];

