ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_Tbl_Account_LoginLevel] DEFAULT ((0)) FOR [LoginLevel];

