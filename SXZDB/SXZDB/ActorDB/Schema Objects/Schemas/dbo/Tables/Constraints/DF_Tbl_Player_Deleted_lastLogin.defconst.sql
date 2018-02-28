ALTER TABLE [dbo].[Tbl_Player_Deleted]
    ADD CONSTRAINT [DF_Tbl_Player_Deleted_lastLogin] DEFAULT (getdate()) FOR [LastLoginTime];

