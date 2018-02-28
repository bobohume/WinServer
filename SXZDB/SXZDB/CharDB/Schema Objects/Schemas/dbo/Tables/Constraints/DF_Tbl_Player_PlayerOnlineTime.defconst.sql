ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_PlayerOnlineTime] DEFAULT ((0)) FOR [TotalOnlineTime];

