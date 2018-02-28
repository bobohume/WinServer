ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_Prentice] DEFAULT ((0)) FOR [IsPrentice];

