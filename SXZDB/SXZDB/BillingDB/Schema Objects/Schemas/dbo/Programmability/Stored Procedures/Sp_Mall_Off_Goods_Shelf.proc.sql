CREATE PROCEDURE [dbo].[Sp_Mall_Off_Goods_Shelf] 
@MallTypeName		VARCHAR(32),			--商城名称
@MainTypeName		VARCHAR(32),			--主分类名称
@PriceRackName		VARCHAR(32),			--商店名称
@GoodsName			VARCHAR(32)				--商品名称

AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
		
	BEGIN TRANSACTION Mall_Off_Goods_Shelf
	
	--这里的错误返回值
	--0 -> 没有错误
	--1 -> 商店不存在
	--2 -> 商品不存在
	--3 -> 商城种类不存在
	DECLARE @ERR INT
	SET @ERR = 0
	
	PRINT N'下架商城物品......'	
	DECLARE @PriceRackID INT
	SET @PriceRackID = 0
	DECLARE @MoneyType INT
	SET @MoneyType = 0
	DECLARE @MainTypeId INT
	SET @MainTypeId = 0
	
	-- 校验商城类型是否合法
	IF @MallTypeName = '元宝商城'
		SET @MoneyType = 1
	ELSE IF @MallTypeName = '绑定元宝商城'
		SET @MoneyType = 2	
	ELSE
	BEGIN
		PRINT N'ERROR 3 -> Mall Type Wrong......'	
		SET @ERR = 3
	END
	
	-- 校验主分类是否合法
	IF @ERR = 0
	BEGIN
		IF @MoneyType = 1
		BEGIN
			IF NOT EXISTS (SELECT 1 FROM Tbl_PriceRack WHERE PriceRackName like @MainTypeName AND (PriceRackID BETWEEN 0 AND 10))
			BEGIN 
				SET @ERR = 4
				PRINT N'ERROR 4 -> The Main Type is not exist......'				
			END
			ELSE
				SELECT @MainTypeId = PriceRackID FROM Tbl_PriceRack WHERE PriceRackName like @MainTypeName AND (PriceRackID BETWEEN 0 AND 10)
		END
		ELSE IF @MoneyType = 2
		BEGIN
			IF NOT EXISTS (SELECT 1 FROM Tbl_PriceRack WHERE PriceRackName like @MainTypeName AND PriceRackID > 10)
			BEGIN 
				SET @ERR = 4
				PRINT N'ERROR 4 -> The Main Type is not exist......'		
			END
			ELSE
				SELECT @MainTypeId = PriceRackID FROM Tbl_PriceRack WHERE PriceRackName like @MainTypeName AND PriceRackID > 10
		END
	END
	
	-- 校验子分类是否合法
	IF @ERR = 0
	BEGIN
		IF NOT EXISTS (SELECT 1 FROM Tbl_PriceRack WHERE PriceRackName like @PriceRackName AND PriceRackFID = @MainTypeId)
		BEGIN 
			SET @ERR = 1
			PRINT N'ERROR 1 -> The shop is not exist......'		
		END
		ELSE
			SELECT @PriceRackID = PriceRackID FROM Tbl_PriceRack WHERE PriceRackName like @PriceRackName AND PriceRackFID = @MainTypeId
	END
	
	-- 校验商品是否存在 并进行下架操作
	IF @ERR = 0
	BEGIN
		IF NOT EXISTS (SELECT 1 FROM Tbl_Price WHERE GoodsID IN (SELECT GoodsID FROM Tbl_Goods WHERE GoodsName LIKE @GoodsName AND Sub = @PriceRackID)
			AND PriceID IN (SELECT PriceID FROM Tbl_PriceTaxon WHERE PriceRackID = @PriceRackID))
		BEGIN
			SET @ERR = 2
			PRINT N'ERROR 2 -> The goods is not exist......'
		END	
		ELSE
		BEGIN
			PRINT N'SUCCESS -> Mall Off Shelf Goods Shelf......'
			UPDATE Tbl_Price SET EndTime = '2012-01-01 00:00:00'
				WHERE GoodsID IN (SELECT GoodsID FROM Tbl_Goods WHERE GoodsName LIKE @GoodsName AND Sub = @PriceRackID)
					AND PriceID IN (SELECT PriceID FROM Tbl_PriceTaxon WHERE PriceRackID = @PriceRackID)
				
			PRINT N'Update Mall DB Version......'
			UPDATE Tbl_Price_Version SET Version = Version + 1, LASTTIME=GETDATE()
		END
	END
	
	IF @ERR = 0
		COMMIT TRANSACTION Mall_Off_Goods_Shelf
	ELSE
		ROLLBACK TRANSACTION Mall_Off_Goods_Shelf

END