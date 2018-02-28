ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_LoginError] DEFAULT ((0)) FOR [LoginError];

