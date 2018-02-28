ALTER TABLE [dbo].[Tbl_TradeGoods]
    ADD CONSTRAINT [DF_Tbl_TradeGold_GoodsNum] DEFAULT ((0)) FOR [PriceNum];

