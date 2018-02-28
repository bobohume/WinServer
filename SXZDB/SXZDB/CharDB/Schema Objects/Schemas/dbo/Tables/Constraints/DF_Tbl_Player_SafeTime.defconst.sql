ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_SafeTime] DEFAULT ((0)) FOR [SafeTime];

