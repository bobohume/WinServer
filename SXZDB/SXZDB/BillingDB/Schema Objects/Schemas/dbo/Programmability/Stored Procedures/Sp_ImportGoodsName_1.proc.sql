CREATE PROCEDURE [dbo].[Sp_ImportGoodsName]

AS
BEGIN
	SET NOCOUNT ON
	UPDATE Tbl_Goods SET GoodsName=C.ItemName FROM 
	(SELECT A.GoodsID,B.ItemName FROM Tbl_Goods_Item A
		INNER JOIN Tbl_Item B ON A.ItemID=B.ItemID) C
	WHERE Tbl_Goods.GoodsID = C.GoodsID
END