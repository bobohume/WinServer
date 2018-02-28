ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_IsRandStrengthens] DEFAULT ((0)) FOR [IsRandStrengthens];

