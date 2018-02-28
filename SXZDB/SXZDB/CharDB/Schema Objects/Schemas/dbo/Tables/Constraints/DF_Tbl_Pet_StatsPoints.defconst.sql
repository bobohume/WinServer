ALTER TABLE [dbo].[Tbl_Pet]
    ADD CONSTRAINT [DF_Tbl_Pet_StatsPoints] DEFAULT ((0)) FOR [StatsPoints];

