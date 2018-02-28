ALTER TABLE [dbo].[Tbl_ScheduleBuff]
    ADD CONSTRAINT [DF_Tbl_ScheduleBuff_IsHit] DEFAULT ((0)) FOR [IsHit];

