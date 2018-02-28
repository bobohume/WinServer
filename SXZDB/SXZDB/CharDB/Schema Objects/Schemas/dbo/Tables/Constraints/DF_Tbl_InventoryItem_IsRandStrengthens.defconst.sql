ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_IsRandStrengthens] DEFAULT ((0)) FOR [IsRandStrengthens];

