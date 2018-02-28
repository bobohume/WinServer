ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_CurWear] DEFAULT ((0)) FOR [CurWear];

