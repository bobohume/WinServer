CREATE TABLE [dbo].[TBL_USER_PURCHASE_HISTORY] (
    [DATE]            DATETIME     NOT NULL,
    [ACCOUNTID]       INT          NOT NULL,
    [PAY_POINTS]      AS           ([LEFT_PAYPOINTS]+[COST_PAYPOINTS]),
    [FREE_POINTS]     AS           ([LEFT_FREEPOINTS]+[COST_FREEPOINTS]),
    [HONOR]           AS           (([LEFT_HONOR]+[COST_HONOR])-[GET_HONOR]),
    [ORDERID]         INT          NOT NULL,
    [COST_PAYPOINTS]  INT          NOT NULL,
    [COST_FREEPOINTS] INT          NOT NULL,
    [COST_HONOR]      INT          NOT NULL,
    [GET_HONOR]       INT          NOT NULL,
    [LEFT_PAYPOINTS]  INT          NOT NULL,
    [LEFT_FREEPOINTS] INT          NOT NULL,
    [LEFT_HONOR]      INT          NOT NULL,
    [IP]              VARCHAR (20) NOT NULL,
    [MAC]             VARCHAR (20) NOT NULL
);

