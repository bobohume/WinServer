ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_LogoutTime] DEFAULT (getdate()) FOR [LogoutTime];

