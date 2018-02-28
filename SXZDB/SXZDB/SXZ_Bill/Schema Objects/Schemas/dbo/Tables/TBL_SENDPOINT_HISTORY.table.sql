CREATE TABLE [dbo].[TBL_SENDPOINT_HISTORY] (
    [DATE]        DATETIME       NOT NULL,
    [ACCOUNTID]   INT            NOT NULL,
    [PAY_POINTS]  INT            NOT NULL,
    [FREE_POINTS] INT            NOT NULL,
    [HONOR]       INT            NOT NULL,
    [MEMO]        NVARCHAR (100) NOT NULL
);

