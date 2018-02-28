/*CREATE PROCEDURE [dbo].[Sp_QueryGoodsPrice]
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
	
	SELECT A.PriceID, A.GoodsID, B.GoodsName, B.ShowItemID, A.DiscountID, B.Store, d.PriceRackID/100 Main, d.PriceRackID Sub,
		A.PriceType, A.Price,C.Rate Rate, A.StartTime, A.EndTime, C.StartTime, C.EndTime 
	FROM TBL_PRICE A 
	JOIN TBL_GOODS B ON A.GOODSID=B.GOODSID
	JOIN TBL_DISCOUNT C ON A.GOODSID=C.GOODSID AND C.StartTime<=@EndTime AND C.EndTime>=@StartTime
	JOIN Tbl_PriceTaxon D ON A.PriceID = D.PriceID
	JOIN Tbl_PriceRack E ON D.PriceRackID= E.PriceRackID
	ORDER BY d.PriceRackID,ISNULL(D.PriceTaxonID,200000)
END*/

