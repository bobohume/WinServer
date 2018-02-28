ALTER TABLE [dbo].[Tbl_Goods]
    ADD CONSTRAINT [DF_Tbl_Goods_Store] DEFAULT ((1)) FOR [Store];

