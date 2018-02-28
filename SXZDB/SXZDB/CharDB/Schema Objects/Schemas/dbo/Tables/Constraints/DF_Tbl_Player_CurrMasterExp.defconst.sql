ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_CurrMasterExp] DEFAULT ((0)) FOR [CurrMasterExp];

