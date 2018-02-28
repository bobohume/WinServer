ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_LoginTime] DEFAULT (getdate()) FOR [LoginTime];

