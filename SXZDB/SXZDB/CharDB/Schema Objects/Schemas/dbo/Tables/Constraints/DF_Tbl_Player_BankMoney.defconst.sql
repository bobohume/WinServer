ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF_Tbl_Player_BankMoney] DEFAULT ((0)) FOR [BankMoney];

