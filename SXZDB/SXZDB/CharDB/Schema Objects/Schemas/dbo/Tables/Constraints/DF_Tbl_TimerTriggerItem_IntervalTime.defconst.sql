ALTER TABLE [dbo].[Tbl_TimerItem]
    ADD CONSTRAINT [DF_Tbl_TimerTriggerItem_IntervalTime] DEFAULT ((0)) FOR [IntervalTime];

