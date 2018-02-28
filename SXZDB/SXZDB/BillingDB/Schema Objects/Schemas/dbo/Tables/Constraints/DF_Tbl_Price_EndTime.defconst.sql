ALTER TABLE [dbo].[Tbl_Price]
    ADD CONSTRAINT [DF_Tbl_Price_EndTime] DEFAULT (getdate()) FOR [EndTime];

