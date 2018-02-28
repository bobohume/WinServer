ALTER TABLE [dbo].[Tbl_Player_Deleted]
    ADD CONSTRAINT [DF_Tbl_Player_Deleted_DeletedDate] DEFAULT (getdate()) FOR [DeletedDate];

