ALTER TABLE [dbo].[Tbl_BankItem]
    ADD CONSTRAINT [DF_Tbl_BankItem_ItemQuantity] DEFAULT ((0)) FOR [Quantity];

