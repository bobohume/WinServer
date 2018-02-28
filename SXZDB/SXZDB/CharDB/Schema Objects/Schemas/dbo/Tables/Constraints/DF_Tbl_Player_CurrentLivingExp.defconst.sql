ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_CurrentLivingExp] DEFAULT ((0)) FOR [CurrentLivingExp];

