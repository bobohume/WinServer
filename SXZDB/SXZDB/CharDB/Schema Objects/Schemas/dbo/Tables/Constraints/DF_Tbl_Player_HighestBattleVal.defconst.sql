ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_HighestBattleVal] DEFAULT ((0)) FOR [HighestBattleVal];