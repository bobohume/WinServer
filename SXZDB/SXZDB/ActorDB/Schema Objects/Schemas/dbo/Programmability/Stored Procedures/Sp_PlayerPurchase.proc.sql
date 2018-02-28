-- =============================================
-- Author:		<LivenHotch>
-- Create date: <2010-05-27>
-- Description:	<内购不在线加钱处理>
-- modifyer:    <Daniel>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_PlayerPurchase]
@GMPLAYERID		INT,
@PLAYERID		INT,
@GOLD			INT,
@PRICE			FLOAT,
@REASON			VARCHAR(64)
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

	DECLARE @CurGold INT
	SET @CurGold = 0
	DECLARE @CurRechargeGold INT
	SET @CurRechargeGold = 0
	DECLARE @ID INT
	SET @ID = 0
	DECLARE @Err INT
	SET @Err = 0

	BEGIN TRANSACTION PlayerPurchase
	SELECT @CurGold = Gold, @CurRechargeGold = DrawGold FROM Tbl_Player WHERE PlayerID = @PLAYERID 
	IF @@ROWCOUNT <> 0
	BEGIN
		SET @CurGold = @CurGold + @GOLD
		SET @CurRechargeGold = @CurRechargeGold + @GOLD
		UPDATE Tbl_Player  SET Gold = @CurGold, DrawGold = @CurRechargeGold WHERE PlayerID = @PLAYERID

		INSERT INTO Tbl_LogPlayer(sourceid,targetid,opttype,val0,val1,val2,val3,val4) 
			VALUES (@GMPLAYERID, @PLAYERID,'金钱操作',@GOLD,@PRICE,@REASON,'','')
	END
	ELSE
		SET @Err = 1

	IF @Err <> 0
		ROLLBACK TRANSACTION PlayerPurchase
	ELSE
		COMMIT TRANSACTION PlayerPurchase

	SELECT @Err AS 'result'

END




