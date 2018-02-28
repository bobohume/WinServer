CREATE TABLE [dbo].[Tbl_Price] (
    [PriceID]    BIGINT   NOT NULL,
    [GoodsID]    INT      NOT NULL,
    [DiscountID] INT      NOT NULL,
    [PriceType]  TINYINT  NOT NULL,
    [Price]      INT      NOT NULL,
    [CreateTime] DATETIME NOT NULL,
    [StartTime]  DATETIME NOT NULL,
    [EndTime]    DATETIME NOT NULL
);

