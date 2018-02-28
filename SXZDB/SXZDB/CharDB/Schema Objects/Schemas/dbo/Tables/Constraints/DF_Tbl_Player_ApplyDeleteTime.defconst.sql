ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_ApplyDeleteTime] DEFAULT (getdate()) FOR [ApplyDeleteTime];

