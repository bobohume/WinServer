ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_CurWear] DEFAULT ((0)) FOR [CurWear];

