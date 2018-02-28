ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_SecondPasswordState] DEFAULT ((0)) FOR [SecondPasswordState];

