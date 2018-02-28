ALTER TABLE [dbo].[Tbl_Price]
    ADD CONSTRAINT [DF_Tbl_Price_CreateTime] DEFAULT (getdate()) FOR [CreateTime];

