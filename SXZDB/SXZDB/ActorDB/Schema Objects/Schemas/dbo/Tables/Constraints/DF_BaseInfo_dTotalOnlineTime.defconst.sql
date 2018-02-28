ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_dTotalOnlineTime] DEFAULT ((0)) FOR [dTotalOnlineTime];

