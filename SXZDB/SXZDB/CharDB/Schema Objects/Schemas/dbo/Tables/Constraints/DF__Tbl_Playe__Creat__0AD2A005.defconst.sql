ALTER TABLE [dbo].[Tbl_Player]
    ADD CONSTRAINT [DF__Tbl_Playe__Creat__0AD2A005] DEFAULT (getdate()) FOR [CreateTime];

