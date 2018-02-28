ALTER TABLE [dbo].[Tbl_Price]
    ADD CONSTRAINT [DF_Tbl_Price_StartTime] DEFAULT (getdate()) FOR [StartTime];

