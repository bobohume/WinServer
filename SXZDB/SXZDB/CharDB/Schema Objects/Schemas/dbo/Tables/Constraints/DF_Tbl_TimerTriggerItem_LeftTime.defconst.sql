ALTER TABLE [dbo].[Tbl_TimerItem]
    ADD CONSTRAINT [DF_Tbl_TimerTriggerItem_LeftTime] DEFAULT ((0)) FOR [LeftTime];

