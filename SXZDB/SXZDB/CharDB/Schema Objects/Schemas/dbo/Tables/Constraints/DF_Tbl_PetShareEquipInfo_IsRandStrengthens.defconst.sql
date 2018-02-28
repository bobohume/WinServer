ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_IsRandStrengthens] DEFAULT ((0)) FOR [IsRandStrengthens];

