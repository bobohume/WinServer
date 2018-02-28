ALTER TABLE [dbo].[Tbl_PetShareEquipInfo]
    ADD CONSTRAINT [DF_Tbl_PetShareEquipInfo_ItemQuantity] DEFAULT ((0)) FOR [Quantity];

