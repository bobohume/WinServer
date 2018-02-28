CREATE PROCEDURE [dbo].[Sp_CreatePrice]

AS
BEGIN
	SET NOCOUNT ON
	INSERT Tbl_Price (PriceID,GoodsID,DiscountID,PriceType,Price,CreateTime,StartTime,EndTime)
	SELECT CAST(GoodsID AS VARCHAR(10))+REPLICATE('0',8-LEN(CAST(GETDATE() AS INT)))+CAST(CAST(GETDATE() AS INT) AS VARCHAR(10)), 
		GoodsID,0, GoodsID/100000000 -1,Price,GETDATE(),GETDATE(),GETDATE()
		FROM Tbl_Goods

	UPDATE Tbl_Price SET DiscountID=A.DiscountID,StartTime=A.StartTime, EndTime=A.EndTime
		FROM Tbl_Discount A WHERE A.GoodsID=Tbl_Price.GoodsID
END