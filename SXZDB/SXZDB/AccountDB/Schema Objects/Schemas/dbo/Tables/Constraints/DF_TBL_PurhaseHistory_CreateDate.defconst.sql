ALTER TABLE [dbo].[TBL_PurhaseHistory]
    ADD CONSTRAINT [DF_TBL_PurhaseHistory_CreateDate] DEFAULT (getdate()) FOR [CreateDate];

