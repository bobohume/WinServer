ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_TotalOnlineTime] DEFAULT ((0)) FOR [TotalOnlineTime];

