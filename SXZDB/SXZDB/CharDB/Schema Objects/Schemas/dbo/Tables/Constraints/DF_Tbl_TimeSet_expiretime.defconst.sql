ALTER TABLE [dbo].[Tbl_TimeSet]
    ADD CONSTRAINT [DF_Tbl_TimeSet_expiretime] DEFAULT ((0)) FOR [expiretime];

