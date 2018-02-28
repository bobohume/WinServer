CREATE TABLE [dbo].[TBL_PurhaseHistory] (
    [transaction_id] VARCHAR (32) NOT NULL,
    [playerid]       INT          NOT NULL,
    [product_id]     VARCHAR (32) NOT NULL,
    [item_id]        VARCHAR (32) NOT NULL,
    [quantity]       INT          NOT NULL,
    [purchase_date]  VARCHAR (32) NOT NULL,
    [gold]           INT          NOT NULL,
    [CreateDate]     DATETIME     NOT NULL
);

