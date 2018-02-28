ALTER TABLE [dbo].[Tbl_Goods]
    ADD CONSTRAINT [DF_Tbl_Billing_Goods_Price] DEFAULT ((0)) FOR [Price];

