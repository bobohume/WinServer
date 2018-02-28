CREATE PROCEDURE [dbo].[Sp_Mall_Del_False_Goods] 
@StartTime		DATETIME,				--商城上架错误的商品
@EndTime		DATETIME				--当时的日期

AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
		
	BEGIN TRANSACTION Mall_Del_False_Goods
	
	PRINT N'删除错误物品......'	
	DELETE FROM Tbl_PriceTaxon 
		WHERE CreateDate BETWEEN @StartTime AND @EndTime
		
	DELETE FROM Tbl_Goods_Item
		WHERE GoodsID IN (SELECT GoodsID FROM Tbl_Discount WHERE CreateTime BETWEEN @StartTime AND @EndTime)

	DELETE FROM TBL_GOODS
		WHERE GoodsID IN (SELECT GoodsID FROM Tbl_Discount WHERE CreateTime BETWEEN @StartTime AND @EndTime)
	
	DELETE FROM Tbl_Price
		WHERE CreateTime BETWEEN @StartTime AND @EndTime
		
	DELETE FROM Tbl_Discount
		WHERE CreateTime BETWEEN @StartTime AND @EndTime
		
	COMMIT TRANSACTION Mall_Del_False_Goods

END