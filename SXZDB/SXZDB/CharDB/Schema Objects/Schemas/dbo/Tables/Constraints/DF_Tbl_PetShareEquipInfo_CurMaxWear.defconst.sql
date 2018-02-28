ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_CurMaxWear] DEFAULT ((0)) FOR [CurMaxWear];

