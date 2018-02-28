ALTER TABLE [dbo].[Tbl_Discount]
    ADD CONSTRAINT [DF_Tbl_Biling_Discount_CreateDate] DEFAULT (getdate()) FOR [CreateTime];

