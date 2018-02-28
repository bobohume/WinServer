ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_EquipStrengthens] DEFAULT ((0)) FOR [EquipStrengthens];

