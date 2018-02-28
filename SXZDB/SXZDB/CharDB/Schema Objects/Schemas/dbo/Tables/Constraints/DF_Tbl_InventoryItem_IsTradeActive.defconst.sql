ALTER TABLE [dbo].[Tbl_InventoryItem]
    ADD CONSTRAINT [DF_Tbl_InventoryItem_IsTradeActive] DEFAULT ((0)) FOR [IsTradeActive];

