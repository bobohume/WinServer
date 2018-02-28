ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_MasterLevel] DEFAULT ((0)) FOR [MasterLevel];

