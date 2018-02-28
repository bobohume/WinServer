ALTER TABLE [dbo].[Tbl_Discount_OLD]
    ADD CONSTRAINT [DF_Tbl_Discount_OLD_BeginDate] DEFAULT (getdate()) FOR [StartTime];

