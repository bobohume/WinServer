CREATE PROCEDURE [dbo].[Sp_QueryGoodsPrice]

AS
BEGIN
	SET NOCOUNT ON
	DECLARE @StartTime DATETIME, @EndTime DATETIME
	SET @StartTime = CAST(YEAR(GETDATE()) AS VARCHAR(5)) +	'-' + CAST(MONTH(GETDATE()) AS VARCHAR(3)) +
			'-' + CAST(DAY(GETDATE()) AS VARCHAR(3))
	SET @EndTime = @StartTime + ' 23:59:59'
	SET @StartTime = @StartTime + ' 00:00:00'

	--查询上架商品所需的道具信息
	SELECT GoodsID, ItemID, ItemNum FROM Tbl_Goods_Item
		WHERE GoodsID IN (SELECT A.GoodsID FROM Tbl_Goods A
			INNER JOIN Tbl_Price B ON A.GoodsID=B.GoodsID
			WHERE B.EndTime>=@StartTime AND B.StartTime<=@EndTime)
	
	SELECT B.PriceID, B.GoodsID, D.GoodsName, D.ShowItemID, B.DiscountID, D.Store, A.PriceRackID/100 Main, A.PriceRackID Sub,
		B.PriceType, B.Price,C.Rate Rate, B.StartTime, B.EndTime, C.StartTime, C.EndTime 
	FROM Tbl_PriceTaxon A 
	INNER JOIN Tbl_Price B ON A.PriceID = B.PriceID AND B.StartTime<=@EndTime AND B.EndTime>=@StartTime
	INNER JOIN TBL_DISCOUNT C ON B.DISCOUNTID=C.DISCOUNTID AND C.StartTime<=@EndTime AND C.EndTime>=@StartTime
	INNER JOIN TBL_GOODS D ON B.GOODSID = D.GOODSID AND C.GOODSID=D.GOODSID
	INNER JOIN Tbl_PriceRack E ON A.PriceRackID= E.PriceRackID
	ORDER BY A.PriceRackID,A.PriceTaxonID
END