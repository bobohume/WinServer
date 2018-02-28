ALTER TABLE [dbo].[Tbl_Account]
    ADD CONSTRAINT [DF_BaseInfo_IsAdult] DEFAULT ((0)) FOR [IsAdult];

