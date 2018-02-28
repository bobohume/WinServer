CREATE TABLE [dbo].[TBL_PLAYER] (
    [ID]         INT          IDENTITY (1, 1) NOT NULL,
    [PlayerID]   INT          NOT NULL,
    [PlayerName] VARCHAR (32) NOT NULL,
    [AccountID]  INT          NOT NULL,
    [Delete]     TINYINT      NOT NULL
);











