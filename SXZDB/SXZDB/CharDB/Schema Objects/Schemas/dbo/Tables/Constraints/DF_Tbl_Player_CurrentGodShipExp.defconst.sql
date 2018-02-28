ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_CurrentGodShipExp] DEFAULT ((0)) FOR [CurrentGodShipExp];