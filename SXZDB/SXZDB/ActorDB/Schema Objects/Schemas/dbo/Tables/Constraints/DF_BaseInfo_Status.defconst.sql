ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_Status] DEFAULT ((0)) FOR [Status];

