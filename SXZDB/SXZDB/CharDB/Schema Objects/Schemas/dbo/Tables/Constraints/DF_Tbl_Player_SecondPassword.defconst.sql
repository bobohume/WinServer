ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_SecondPassword] DEFAULT ('') FOR [SecondPassword];

