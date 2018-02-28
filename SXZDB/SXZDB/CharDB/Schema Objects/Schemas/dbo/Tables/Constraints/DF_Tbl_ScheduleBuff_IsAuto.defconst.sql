ALTER TABLE [dbo].[Tbl_ScheduleBuff]
    ADD CONSTRAINT [DF_Tbl_ScheduleBuff_IsAuto] DEFAULT ((0)) FOR [IsAuto];

