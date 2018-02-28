ALTER TABLE [dbo].[Tbl_Mount]
    ADD CONSTRAINT [DF_Tbl_Mount_Status] DEFAULT ((0)) FOR [Status];

