ALTER TABLE [dbo].[Tbl_BankItem]
    ADD CONSTRAINT [DF_Tbl_BankItem_LapseTime] DEFAULT ((0)) FOR [LapseTime];

