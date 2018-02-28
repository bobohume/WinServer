ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_Quality] DEFAULT ((0)) FOR [Quality];

