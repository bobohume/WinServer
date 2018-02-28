-- =============================================
-- Author:		Daniel
-- Create date: <2011-3-1>
-- Description:	<消费帐号充值元宝>
-- =============================================
CREATE PROCEDURE [dbo].[USP_COSTGOLD_SXZ]
@AccountID INT,
@CostGold INT
AS
BEGIN
	SET NOCOUNT ON
	DECLARE @ERR INT
	DECLARE @GOLD INT
	SET @ERR = 0
	SET @GOLD = 0
	BEGIN TRANSACTION COSTGOLD
	SELECT @GOLD=PAY_POINTS FROM T_USER_PAY WHERE ACCOUNTID = @AccountID
	IF @GOLD >= @CostGold
	BEGIN
		SET @GOLD = @GOLD - @CostGold
		UPDATE T_USER_PAY SET PAY_POINTS=@Gold WHERE ACCOUNTID = @AccountID
		COMMIT TRANSACTION COSTGOLD
		SET @ERR = 1
	END
	
	IF @ERR <> 1
		ROLLBACK TRANSACTION COSTGOLD

	SELECT @ERR, @GOLD
END






