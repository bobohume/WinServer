ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_LockDivorceTime] DEFAULT ((0)) FOR [LockDivorceTime];

