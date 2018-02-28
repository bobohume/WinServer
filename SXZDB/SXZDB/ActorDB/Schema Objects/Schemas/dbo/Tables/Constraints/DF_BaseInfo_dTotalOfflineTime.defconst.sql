ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_dTotalOfflineTime] DEFAULT ((0)) FOR [dTotalOfflineTime];

