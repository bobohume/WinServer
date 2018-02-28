ALTER TABLE [dbo].[Tbl_TimerItem]
    ADD CONSTRAINT [DF_Tbl_TimerTriggerItem_State] DEFAULT ((0)) FOR [State];

