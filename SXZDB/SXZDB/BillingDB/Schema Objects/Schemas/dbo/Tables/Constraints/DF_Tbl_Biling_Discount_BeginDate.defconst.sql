ALTER TABLE [dbo].[Tbl_Discount]
    ADD CONSTRAINT [DF_Tbl_Biling_Discount_BeginDate] DEFAULT (getdate()) FOR [StartTime];

