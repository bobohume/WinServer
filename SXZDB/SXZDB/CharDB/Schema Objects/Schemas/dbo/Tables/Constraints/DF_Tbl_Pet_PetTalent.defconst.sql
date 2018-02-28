ALTER TABLE [dbo].[Tbl_Pet]
    ADD CONSTRAINT [DF_Tbl_Pet_PetTalent] DEFAULT ((0)) FOR [PetTalent];

