ALTER TABLE [dbo].[Tbl_BankItem]
    ADD CONSTRAINT [DF_Tbl_BankItem_IsEquip] DEFAULT ((0)) FOR [IsEquip];

