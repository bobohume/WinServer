ALTER TABLE [dbo].[Tbl_Player_Deleted]
    ADD CONSTRAINT [DF_Tbl_Player_Deleted_AccountId] DEFAULT ((0)) FOR [AccountID];

