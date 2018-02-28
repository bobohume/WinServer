ALTER TABLE [dbo].[Tbl_ScheduleBuff]
    ADD CONSTRAINT [DF_Tbl_ScheduleBuff_IsVisible] DEFAULT ((0)) FOR [IsVisible];

