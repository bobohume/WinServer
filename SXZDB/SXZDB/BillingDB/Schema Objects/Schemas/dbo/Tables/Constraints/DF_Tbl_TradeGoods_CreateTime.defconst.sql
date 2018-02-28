ALTER TABLE [dbo].[Tbl_TradeGoods]
    ADD CONSTRAINT [DF_Tbl_TradeGoods_CreateTime] DEFAULT (getdate()) FOR [CreateTime];

