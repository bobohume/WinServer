CREATE PROCEDURE Sp_QueryGoodsPriceEx
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

    DECLARE @StartTime DATETIME, @EndTime DATETIME
	SET @StartTime = CAST(YEAR(GETDATE()) AS VARCHAR(5)) +	'-' + CAST(MONTH(GETDATE()) AS VARCHAR(3)) +
			'-' + CAST(DAY(GETDATE()) AS VARCHAR(3))
	SET @EndTime = @StartTime + ' 23:59:59'
	SET @StartTime = @StartTime + ' 00:00:00'

	SELECT B.GoodsName, B.ShowItemID, A.Price,
	CASE B.Store WHEN 1 THEN '元宝商城' ELSE '绑定元宝商城' END,
	D.PriceRackName, E.PriceRackName
	FROM TBL_PRICE A 
	INNER JOIN TBL_GOODS B 
		ON A.GOODSID=B.GOODSID AND B.StartTime<=@EndTime AND B.EndTime>=@StartTime
	JOIN Tbl_PriceTaxon C
		ON A.PriceID = C.PriceID
	LEFT JOIN Tbl_PriceRack D ON D.PriceRackID = C.PriceRackID/100
	LEFT JOIN Tbl_PriceRack E ON E.PriceRackID = C.PriceRackID
	ORDER BY C.PriceRackID,ISNULL(C.PriceTaxonID,200000)
END