-- =============================================
-- Author:		<ZhouCheng>
-- Create date: <2011-03-31>
-- Description:	<导入商品折扣信息>
-- =============================================
/*CREATE PROC [Sp_InputDiscountInfo]
(
	@DiscountID		INT,		--折扣配置编号
	@GoodsID		INT,		--商品编号
	@Rate			TinyInt,	--折扣值
	@StartTime		DATETIME,	--折扣开始时间
	@EndTime		DATETIME	--折扣结束时间
)
AS 
BEGIN
	SET NOCOUNT ON 
	SET XACT_ABORT ON
	DECLARE @ISERR INT
	SELECT @ISERR = -1 -- 该商品折扣编号存在错误
	BEGIN TRANSACTION ITDT
	IF NOT EXISTS(SELECT 1 FROM Tbl_Discount WHERE DiscountID = @DiscountID)
	BEGIN
		IF @StartTime >= @EndTime
			SELECT @ISERR = -2	-- 该商品折扣上架日期大于等于下架日期
		ELSE
		BEGIN
			IF NOT EXISTS(SELECT 1 FROM Tbl_Goods WHERE GoodsID = @GoodsID)
				SELECT @ISERR = -3	-- 该商品不存在
			ELSE
			BEGIN			
				IF EXISTS(SELECT 1 FROM Tbl_Discount WHERE GoodsID = @GoodsID
					AND ((StartTime >= @StartTime AND StartTime < @EndTime)
							OR(EndTime > @StartTime AND EndTime <= @EndTime)))
					SELECT @ISERR = -4	-- 该商品的打折时间区间存在重复数据
				ELSE
				BEGIN
					IF @Rate > 10 OR @Rate < 0
						SELECT @ISERR = -5	-- 该商品的折扣率错误
					ELSE
					BEGIN
						BEGIN
							INSERT Tbl_Discount(DiscountID,GoodsID,Rate,StartTime,EndTime)
								SELECT @DiscountID,@GoodsID,@Rate,@StartTime,@EndTime

							INSERT TBL_Price(PriceID,GoodsID,DiscountID,Price,PriceType,StartTime,EndTime)
								SELECT CAST(@GoodsID as VARCHAR(20))+CAST(@DiscountID as VARCHAR(20)),
										@GoodsID,@DiscountID,@Rate*Price/10,Store,StartTime,EndTime
								FROM Tbl_Goods WHERE GoodsID = @GoodsID
							SELECT @ISERR = 0
						END
					END
				END
			END
		END
	END
	
	IF @ISERR = 0
		COMMIT TRANSACTION ITDT
	ELSE
		ROLLBACK TRANSACTION ITDT
	RETURN @ISERR
END*/
