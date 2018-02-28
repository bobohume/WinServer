ALTER TABLE [dbo].[Tbl_Price]
    ADD CONSTRAINT [DF_Tbl_Price_PriceType] DEFAULT ((0)) FOR [PriceType];

