ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_LivingLevel] DEFAULT ((0)) FOR [LivingLevel];

