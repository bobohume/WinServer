ALTER TABLE [dbo].[Tbl_Discount]
    ADD CONSTRAINT [DF_Tbl_Biling_Discount_EndDate] DEFAULT (getdate()) FOR [EndTime];

