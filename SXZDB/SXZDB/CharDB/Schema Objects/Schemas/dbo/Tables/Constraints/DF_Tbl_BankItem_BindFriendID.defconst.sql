ALTER TABLE [dbo].[Tbl_BankItem]
    ADD CONSTRAINT [DF_Tbl_BankItem_BindFriendID] DEFAULT ((0)) FOR [BindFriendID];

