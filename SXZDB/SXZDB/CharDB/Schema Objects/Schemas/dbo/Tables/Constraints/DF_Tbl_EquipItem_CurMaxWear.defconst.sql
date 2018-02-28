ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_CurMaxWear] DEFAULT ((0)) FOR [CurMaxWear];

