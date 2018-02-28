ALTER TABLE [dbo].[Tbl_Debuff]
    ADD CONSTRAINT [DF_Tbl_DeBuff_Self] DEFAULT ((0)) FOR [Self];

