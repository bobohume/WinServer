ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_NewSafeTimeDate] DEFAULT (getdate()) FOR [NewSafeTimeDate];

