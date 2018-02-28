CREATE TABLE [dbo].[Tbl_TradeGoods] (
    [TradeGoodsID] BIGINT   IDENTITY (1, 1) NOT NULL,
    [AccountID]    INT      NOT NULL,
    [PlayerID]     INT      NOT NULL,
    [PriceID]      BIGINT   NOT NULL,
    [PriceNum]     INT      NOT NULL,
    [PriceType]    TINYINT  NOT NULL,
    [Price]        INT      NOT NULL,
    [ServerID]     INT      NOT NULL,
    [CreateTime]   DATETIME NOT NULL
);

