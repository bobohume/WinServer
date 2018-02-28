-- =============================================
-- Author:		<ZhouCheng>
-- Create date: <2011-03-31>
-- Description:	<导入商品信息>
-- =============================================
/*CREATE PROCEDURE [dbo].[Sp_InputGoodsInfo]
@GoodsID INT, @GoodsName VARCHAR (32), @Store TINYINT, @Main TINYINT, @Sub TINYINT, @Price INT, @StartTime DATETIME, @EndTime DATETIME, @ItemList VARCHAR (MAX)
AS
BEGIN
	SET NOCOUNT ON 
	SET XACT_ABORT ON
	DECLARE @ISERR INT
	DECLARE @SQL VARCHAR(MAX)
	SELECT @SQL = ''
	BEGIN TRANSACTION ITDT
	SELECT @ISERR = -1	-- 商品编号已存在
	IF NOT EXISTS(SELECT * FROM Tbl_Goods WHERE GoodsID = @GoodsID)
	BEGIN
		IF @StartTime >= @EndTime
			SELECT @ISERR = -2	-- 商品上架日期大于等于下架日期
		ELSE
		BEGIN
			INSERT Tbl_Goods(GoodsID,GoodsName,Store,ShowItemID,Main,Sub,Price,StartTime,EndTime)
				SELECT @GoodsID,@GoodsName,@Store,0,@Main,@Sub,@Price,@StartTime,@EndTime
			SELECT @SQL = 'INSERT Tbl_Goods_Item (GoodsID,ItemID,ItemNum) SELECT '+CAST(@GoodsID AS VARCHAR(MAX))+','+REPLACE(@ITEMLIST,';',' UNION ALL SELECT '+CAST(@GoodsID AS VARCHAR(MAX))+',')
			EXEC(@SQL)
			SELECT @ISERR = 0
		END
	END
	IF @ISERR = 0
		COMMIT TRANSACTION ITDT
	ELSE
		ROLLBACK TRANSACTION ITDT
	RETURN @ISERR
END*/
