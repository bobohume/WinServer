ALTER TABLE [dbo].[Tbl_ScheduleBuff]
    ADD CONSTRAINT [DF_Tbl_ScheduleBuff_NeedTimes] DEFAULT ((0)) FOR [NeedTimes];

