ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_CurrentExp] DEFAULT ((0)) FOR [CurrentExp];

