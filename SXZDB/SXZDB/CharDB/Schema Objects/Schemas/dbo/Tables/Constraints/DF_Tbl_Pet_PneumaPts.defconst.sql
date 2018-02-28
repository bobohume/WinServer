ALTER TABLE [dbo].[Tbl_Pet]
    ADD CONSTRAINT [DF_Tbl_Pet_PneumaPts] DEFAULT ((0)) FOR [PneumaPts];

