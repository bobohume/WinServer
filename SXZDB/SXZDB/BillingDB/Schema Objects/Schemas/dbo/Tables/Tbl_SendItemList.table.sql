CREATE TABLE [dbo].[Tbl_SendItemList] (
    [ID]            INT      IDENTITY (1, 1) NOT NULL,
    [AccountID]     INT      NULL,
    [PlayerID]      INT      NULL,
    [SumPrice]      INT      NULL,
    [SendItemCount] INT      NULL,
    [SendMailCount] INT      NULL,
    [FCreateTime]   DATETIME NULL,
    [LCreateTime]   DATETIME NULL,
    PRIMARY KEY CLUSTERED ([ID] ASC) WITH (ALLOW_PAGE_LOCKS = ON, ALLOW_ROW_LOCKS = ON, PAD_INDEX = OFF, IGNORE_DUP_KEY = OFF, STATISTICS_NORECOMPUTE = OFF)
);

