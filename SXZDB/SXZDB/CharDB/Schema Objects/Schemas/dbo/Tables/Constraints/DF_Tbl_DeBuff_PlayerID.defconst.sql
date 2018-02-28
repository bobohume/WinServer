ALTER TABLE [dbo].[Tbl_Debuff]
    ADD CONSTRAINT [DF_Tbl_DeBuff_PlayerID] DEFAULT ((0)) FOR [PlayerID];

