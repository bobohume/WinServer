ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_GMFlag] DEFAULT ((0)) FOR [GMFlag];

