CREATE PROCEDURE [dbo].[Sp_SaveTradeGoods]
@AccountID INT, @PlayerID INT, @PriceID BIGINT, @PriceNum INT, @PriceType TINYINT, @Price INT, @ServerID INT, @CreateTime DATETIME
AS
BEGIN
    SET NOCOUNT ON
    SET XACT_ABORT ON
    DECLARE @TradeGoodsID BIGINT
    BEGIN TRANSACTION SEND_ITEM
       INSERT INTO [Tbl_TradeGoods]
              ([AccountID],[PlayerID],[PriceID],[PriceNum],
              [PriceType],[Price], [ServerID],[CreateTime])
        VALUES
              (@AccountID, @PlayerID, @PriceID,@PriceNum,
              @PriceType, @Price, @ServerID, @CreateTime)
       SELECT @TradeGoodsID = @@IDENTITY
       DECLARE @SumPrice INT
       SELECT @SumPrice = @PriceNum * @Price
       IF @PriceType = 0 AND @PriceID <> 1000001401107221139 AND GETDATE()<'2012-06-23'
           EXEC SP_SendItem 
              @AccountID = @AccountID 
              ,@PlayerID = @PlayerID 
              ,@SumPrice = @SumPrice
    COMMIT TRANSACTION SEND_ITEM
    SELECT @TradeGoodsID
END