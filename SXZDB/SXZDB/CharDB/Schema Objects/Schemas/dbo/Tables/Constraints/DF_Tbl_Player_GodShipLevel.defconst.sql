ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_GodShipLevel] DEFAULT ((0)) FOR [GodShipLevel];