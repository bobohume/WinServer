-- =============================================
-- Author:		<Daniel>
-- Create date: <2011-03-31>
-- Description:	<查询推荐商品>
-- =============================================
/*CREATE PROCEDURE [dbo].[Sp_QueryRecommend]
@StartTime	DATETIME,		--上架时间
@EndTime	DATETIME		--下架时间
AS
BEGIN
	SET NOCOUNT ON
	
	SELECT C.RecommendID, B.PriceID FROM Tbl_Goods A
		INNER JOIN Tbl_Price B ON A.GoodsID = B.GoodsID
		INNER JOIN Tbl_Recommend C ON A.GoodsID = C.GoodsID
		WHERE A.EndTime>=@StartTime AND A.StartTime<=@EndTime
END*/