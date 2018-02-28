ALTER TABLE [dbo].[Tbl_Price_OLD]
    ADD CONSTRAINT [DF_Tbl_Price_OLD_PriceType] DEFAULT ((0)) FOR [PriceType];

