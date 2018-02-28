ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_LoginIP] DEFAULT ('') FOR [LoginIP];

