ALTER TABLE [dbo].[Tbl_EquipItem]
    ADD CONSTRAINT [DF_Tbl_EquipItem_IsTradeActive] DEFAULT ((0)) FOR [IsTradeActive];

