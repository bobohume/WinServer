ALTER TABLE [dbo].[Tbl_Player_Deleted]
    ADD CONSTRAINT [DF_Tbl_Player_Deleted_BLOBData] DEFAULT (0x00) FOR [BLOBData];

