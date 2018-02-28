ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_ItemQuantity] DEFAULT ((0)) FOR [Quantity];

