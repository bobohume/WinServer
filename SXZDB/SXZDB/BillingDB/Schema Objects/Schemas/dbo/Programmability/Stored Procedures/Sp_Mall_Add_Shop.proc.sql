CREATE PROCEDURE [dbo].[Sp_Mall_Add_Shop] 
@PriceRackID		INT,				--新商店的子编号
@PriceRackName		NVARCHAR(20),		--新商店的名称
@PriceRackFID		INT,				--新商店的主编号
@PirceRackLv		INT					--新商店大分类

AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
		
	BEGIN TRANSACTION Mall_Add_Shop
	
	--这里的错误返回值
	-- 0 -> 没有错误
	-- 1 -> 商店编号重复
	-- 2 ->	商店名重复
	-- 3 -> 子编号和主编号不匹配
	-- 4 -> 商店大类型有误
	DECLARE @ERR INT
	SET @ERR = 0
	
	PRINT N'Adding New Shop......'
	-- 检测主编号和子编号是否匹配
	IF @PriceRackFID <> @PriceRackID/100
	BEGIN
		PRINT N'ERROR 3 -> PriceRackID AND PriceRackFID not match......'
		SET @ERR = 3
	END	
		
	-- 检测主编号和子编号是否匹配
	IF @PirceRackLv <> 1 AND @PirceRackLv <> 2
	BEGIN
		PRINT N'ERROR 4 -> @PirceRackLv Wrong......'
		SET @ERR = 4
	END	
	
	IF @ERR = 0
	BEGIN
		-- 检测商店ID是否重复
		IF EXISTS (SELECT 1 FROM Tbl_PriceRack WHERE PriceRackID = @PriceRackID)
		BEGIN
			PRINT N'ERROR 1 -> PriceRackID aready exist......'
			SET @ERR = 1
		END
		ELSE
		BEGIN
			-- 检测商店名是否重复
			IF @PriceRackID > 10
			BEGIN
				IF EXISTS (SELECT 1 FROM Tbl_PriceRack WHERE PriceRackName = @PriceRackName AND PriceRackFID > 10)
				BEGIN
					PRINT N'ERROR 2 -> PriceRackName aready exist......'
					SET @ERR = 2
				END
			END
			ELSE
			BEGIN
				IF EXISTS (SELECT 1 FROM Tbl_PriceRack WHERE PriceRackName = @PriceRackName AND PriceRackFID < 10 AND PriceRackFID > 0)
				BEGIN
					PRINT N'ERROR 2 -> PriceRackName aready exist......'
					SET @ERR = 2
				END
			END
		END
	END
	
	-- 插入新纪录
	IF @ERR = 0
	BEGIN
		PRINT N'SUCCESS -> ADD Shop......'
		INSERT INTO Tbl_PriceRack (PriceRackID,PriceRackName,PriceRackFID,PirceRackLv)
			SELECT @PriceRackID,@PriceRackName,@PriceRackFID,@PirceRackLv
	END
	
	IF @ERR = 0
		COMMIT TRANSACTION Mall_Add_Shop
	ELSE
		ROLLBACK TRANSACTION Mall_Add_Shop
	
END