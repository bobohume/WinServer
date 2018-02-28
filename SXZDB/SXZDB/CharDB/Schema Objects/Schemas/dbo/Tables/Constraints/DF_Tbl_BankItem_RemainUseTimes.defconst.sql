ALTER TABLE [dbo].[Tbl_BankItem]
    ADD CONSTRAINT [DF_Tbl_BankItem_RemainUseTimes] DEFAULT ((0)) FOR [RemainUseTimes];

