ALTER TABLE [dbo].[Tbl_Discount_OLD]
    ADD CONSTRAINT [DF_Tbl_Discount_OLD_EndDate] DEFAULT (getdate()) FOR [EndTime];

