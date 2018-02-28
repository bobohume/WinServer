ALTER TABLE [dbo].[Tbl_TimerItem]
    ADD CONSTRAINT [DF_Tbl_TimerTriggerItem_TimeFlag] DEFAULT ((0)) FOR [TimeFlag];

