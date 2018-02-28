CREATE PROCEDURE [dbo].[Sp_Mall_Add_Goods] 
@NewGoodsAdd		VARCHAR(MAX)			--新的商城上架物品
--@OldGoodsNoMove		VARCHAR(MAX)			--旧的依旧排在前面的商城上架物品

AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
		
	BEGIN TRANSACTION Mall_Add_Goods
	
	--这里的错误返回值
	-- 0 -> 没有错误
	-- 1 -> 销售的元宝商城或者绑定元宝商城错误
	-- 2 -> 新增物品主分类有问题
	-- 3 -> 新增物品子分类有问题
	-- 4 -> 折扣率是否正确
	-- 5 -> 主次分类是否匹配以及货币类型和主次分类是否匹配
	
	DECLARE @ERR INT
	SET @ERR = 0
	DECLARE @GOODSID INT
	SET @GOODSID = 0
	DECLARE @DISCOUNTID INT
	SET @DISCOUNTID = 0

	-- 上架物品清单
	CREATE TABLE #NEWGOODS (
		ID				INT	PRIMARY KEY,	--编号ID显示顺序(主键)
		MoneyTypeName	NVARCHAR(20),		--物品所在销售的商城
		MainTypeName	NVARCHAR(20),		--物品主分类名称
		SubTypeName		NVARCHAR(20),		--物品小分类名称
		GoodsName		NVARCHAR(32),		--物品名称
		ItemID			INT,				--获得的物品ID
		Price			INT,				--物品价格
		ItemNum			INT,				--物品数量
		ShowItemID		INT,				--物品显示
		Rate			INT,				--物品折扣率
		StartTime		DATETIME,			--物品上架的开始时间
		MoneyType		INT,				--所在的销售商城
		MainType		INT,				--物品大致分类
		SubType			INT,				--物品小类别
		GoodsID			INT,				--商品ID（之后联合计算的
		DiscountID		INT,				--商品折扣ID
		PriceID			BIGINT,				--价格ID（之后联合计算的）
		Temp1			INT,				--临时存储字段1
		Temp2			INT,				--临时存储字段2
		)

	-- ==========================================================================
	-- 下面添加数据---上面不要管了
	-- ==========================================================================
	SET @NewGoodsAdd = REPLACE(@NewGoodsAdd, '##', ',0,0,0,0,0,0,0,0 UNION ALL ')
	SET @NewGoodsAdd = 'INSERT INTO #NEWGOODS (ID,MoneyTypeName,MainTypeName,SubTypeName,GoodsName,ItemID,Price,ItemNum,ShowItemID,Rate,StartTime,MoneyType,MainType,SubType,GoodsID,DiscountID,PriceID,Temp1,Temp2) ' + @NewGoodsAdd
	SET @NewGoodsAdd = @NewGoodsAdd + ',0,0,0,0,0,0,0,0'
	EXECUTE(@NewGoodsAdd)

	-- ==========================================================================
	-- 上面添加数据---下面不要管了
	-- 下面是检测合法性和计算各个ID
	-- ==========================================================================
	-- 检测消费的货币类型是否正确
	IF @ERR = 0
	BEGIN
		SELECT MoneyTypeName FROM #NEWGOODS WHERE MoneyTypeName NOT IN ('元宝商城','绑定元宝商城')
		
		IF @@ROWCOUNT > 0
		BEGIN
			PRINT N'ERROR 1 -> Money Type Error......'
			SET @ERR = 1
		END
	END
	
	-- 检测主分类是否正确
	IF @ERR = 0
	BEGIN
		SELECT MainTypeName FROM #NEWGOODS WHERE MainTypeName NOT IN (SELECT PriceRackName FROM Tbl_PriceRack)
		
		IF @@ROWCOUNT > 0
		BEGIN
			PRINT N'ERROR 2 -> MainType Error......'
			SET @ERR = 2
		END
	END
	
	-- 检测子分类是否正确
	IF @ERR = 0
	BEGIN
		SELECT SubTypeName FROM #NEWGOODS WHERE SubTypeName NOT IN (SELECT PriceRackName FROM Tbl_PriceRack)
		
		IF @@ROWCOUNT > 0
		BEGIN
			PRINT N'ERROR 3 -> SubType Error......'
			SET @ERR = 3
		END
	END
	
	-- 检测折扣率是否正确
	IF @ERR = 0
	BEGIN
		SELECT Rate FROM #NEWGOODS WHERE Rate <= 0 AND Rate > 100
		
		IF @@ROWCOUNT > 0
		BEGIN
			PRINT N'ERROR 4 -> Price rate Error......'
			SET @ERR = 4
		END
	END

	-- 查询主分类和子分类
	IF @ERR = 0
	BEGIN
		UPDATE #NEWGOODS SET MoneyType = 1 WHERE MoneyTypeName LIKE '元宝商城'
		UPDATE #NEWGOODS SET MoneyType = 2 WHERE MoneyTypeName LIKE '绑定元宝商城'
		UPDATE #NEWGOODS SET MainType = (SELECT MIN(PriceRackID) FROM dbo.Tbl_PriceRack WHERE PriceRackName like MainTypeName) WHERE MoneyType = 1
		UPDATE #NEWGOODS SET MainType = (SELECT MAX(PriceRackID) FROM dbo.Tbl_PriceRack WHERE PriceRackName like MainTypeName) WHERE MoneyType = 2
		UPDATE #NEWGOODS SET SubType = (SELECT MIN(PriceRackID) FROM dbo.Tbl_PriceRack WHERE PriceRackName like SubTypeName) WHERE MoneyType = 1
		UPDATE #NEWGOODS SET SubType = (SELECT MAX(PriceRackID) FROM dbo.Tbl_PriceRack WHERE PriceRackName like SubTypeName) WHERE MoneyType = 2
	END	
	
	-- 检测主次分类是否匹配以及货币类型和主次分类是否匹配
	IF @ERR = 0
	BEGIN
		SELECT Rate FROM #NEWGOODS WHERE MainType <> SubType/100 OR (MoneyType = 1 AND MainType > 10) OR (MoneyType = 2 AND MainType < 10)
 		
		IF @@ROWCOUNT > 0
		BEGIN
			PRINT N'ERROR 5 -> Money Type Or Shop Type Not Match......'
			SET @ERR = 5
		END
	END	
	
	
	-- 查询配置 物品ID-折扣ID-价格ID
	IF @ERR = 0
	BEGIN
		SELECT @GOODSID = MAX(GOODSID)+1 FROM TBL_GOODS
		UPDATE #NEWGOODS SET Temp1 = @GOODSID
		UPDATE #NEWGOODS SET GoodsID = Temp1 + ID
		
		SET @DISCOUNTID = CAST((CONVERT(CHAR(6),GETDATE(),12) + '1000') AS INT)
		UPDATE #NEWGOODS SET Temp2 = @DISCOUNTID
		UPDATE #NEWGOODS SET DiscountID = Temp2 + ID	
		
		UPDATE #NEWGOODS SET PriceID = CAST(CAST(GoodsID AS VARCHAR(12))+ CAST(DiscountID AS VARCHAR(12)) AS BIGINT)
	END	
	
	-- 这个测试下是不是计算的正确
	--SELECT * FROM #NEWGOODS
	-- ==========================================================================
	-- 下面是数据库操作--入表
	-- ==========================================================================
	PRINT N'新增商城商品仓库物品数据......'
	INSERT INTO Tbl_Goods (GoodsId,GoodsName,Store,Main,Sub,ShowItemId,Price) 
		SELECT GoodsID,GoodsName,MoneyType,MainType,SubType,ShowItemID,Price FROM #NEWGOODS

	PRINT N'新增商城商品子表数据......'
	INSERT INTO TBL_GOODS_ITEM(GOODSID,ITEMID,ITEMNUM)
		SELECT GoodsID,ItemID,ItemNum FROM #NEWGOODS

	PRINT N'新增商城上架商品折扣数据......'
	INSERT INTO TBL_DISCOUNT(DiscountID,GoodsID,Rate,StartTime,EndTime,CreateTime)
		SELECT DiscountID,GoodsID,Rate,StartTime,'2020-12-30 00:00:00.000',GETDATE() FROM #NEWGOODS

	PRINT N'新增商城上架商品数据......'
	INSERT INTO TBL_PRICE(PRICEID,GOODSID,DiscountID,PriceType,Price,CREATETIME,STARTTIME,ENDTIME)
		SELECT PriceID,GoodsID,DiscountID,MoneyType-1,Price*Rate/100,GETDATE(),StartTime,'2020-12-30 00:00:00.000' FROM #NEWGOODS

	PRINT N'新增商城上架商品排在首位......'
	UPDATE Tbl_PriceTaxon SET PriceTaxonId = PriceTaxonId + 1
	INSERT INTO Tbl_PriceTaxon (PriceTaxonId,PriceRackID,PRICEID,CreateDate)
	SELECT 0,SubType,PriceID,GETDATE() FROM #NEWGOODS
	
	PRINT N'更新商城数据版本标识......'
	UPDATE Tbl_Price_Version SET Version = Version + 1, LASTTIME=GETDATE()

	DROP TABLE #NEWGOODS
	IF @ERR = 0	
		COMMIT TRANSACTION Mall_Add_Goods
	ELSE
		ROLLBACK TRANSACTION Mall_Add_Goods
END