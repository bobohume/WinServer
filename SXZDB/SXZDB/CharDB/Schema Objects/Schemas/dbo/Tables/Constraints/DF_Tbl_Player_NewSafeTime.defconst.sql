ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_NewSafeTime] DEFAULT ((0)) FOR [NewSafeTime];

