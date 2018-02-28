ALTER TABLE [dbo].[Tbl_TradeGoods]
    ADD CONSTRAINT [DF_Tbl_TradeGoods_PriceType] DEFAULT ((0)) FOR [PriceType];

