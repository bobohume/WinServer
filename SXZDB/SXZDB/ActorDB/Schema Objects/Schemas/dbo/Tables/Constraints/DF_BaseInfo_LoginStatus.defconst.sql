ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_LoginStatus] DEFAULT ((0)) FOR [LoginStatus];

