-- =============================================
-- Author:		<Natural>
-- Create date: <2013-11-27>
-- Description:	<Appstore 购买数据验证记录>
-- =============================================
CREATE PROCEDURE [dbo].[USP_VertifyRecord]
@playerid			INT,
@quantity			INT,
@transaction_id		VARCHAR (32),
@product_id			VARCHAR (32),
@item_id			VARCHAR (32),
@purchase_date		VARCHAR (32)

AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON
	DECLARE @ERR INT
	SET @ERR = 0
	DECLARE @GOLD INT
	SET @GOLD = 0
	DECLARE @Money INT
	SET @Money = 0
	DECLARE @CopperCoil INT
	SET @CopperCoil = 0
	DECLARE @SilverCoil INT
	SET @SilverCoil = 0
	DECLARE @GoldCoil INT
	SET @GoldCoil = 0
	
	IF NOT EXISTS(SELECT 1 FROM TBL_PurhaseConf WHERE product_id = @product_id AND item_id = @item_id)
	BEGIN
		SET @ERR = 1001;
	END
	
	IF @ERR = 0
	BEGIN
		IF EXISTS(SELECT 1 FROM TBL_PurhaseHistory WHERE transaction_id = @transaction_id)
		BEGIN
			SET @ERR = 1002;
		END
	END
	
	IF @ERR = 0
	BEGIN
		SELECT @GOLD = Gold, @Money = [Money], @CopperCoil = CopperCoil, @SilverCoil = SilverCoil, @GoldCoil = GoldCoil FROM TBL_PurhaseConf WHERE product_id = @product_id AND item_id = @item_id
		SET @GOLD = @GOLD * @quantity;
		
		INSERT INTO TBL_PurhaseHistory(transaction_id,playerid, product_id, item_id,quantity,purchase_date,gold,CreateDate) 
			SELECT @transaction_id, @playerid, @product_id, @item_id, @quantity, @purchase_date, @GOLD,GETDATE()
	END

	SELECT @ERR,@GOLD,@Money,@CopperCoil,@SilverCoil,@GoldCoil,@transaction_id;
END