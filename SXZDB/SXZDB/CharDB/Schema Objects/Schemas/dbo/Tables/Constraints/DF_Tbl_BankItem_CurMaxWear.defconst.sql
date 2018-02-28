ALTER TABLE [dbo].[Tbl_BankItem]
    ADD CONSTRAINT [DF_Tbl_BankItem_CurMaxWear] DEFAULT ((0)) FOR [CurMaxWear];

