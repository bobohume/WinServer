ALTER TABLE [dbo].[Tbl_Price]
    ADD CONSTRAINT [DF_Tbl_Price_DiscountID] DEFAULT ((0)) FOR [DiscountID];

