-- =============================================
-- Author:		<LivenHotch>
-- Create date: <2010-05-27>
-- Description:	<内购不在线加钱处理>
-- modifyer:    <Daniel>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdatePlayerGold]
@PLAYERID		INT,
@GOLD			INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

	DECLARE @CurGold INT
	SET @CurGold = 0
	DECLARE @ID INT
	SET @ID = 0
	DECLARE @Err INT
	SET @Err = 0

	BEGIN TRANSACTION UpdatePlayerGold
	SELECT @CurGold = Gold FROM Tbl_Player WHERE PlayerID = @PLAYERID 
	IF @@ROWCOUNT <> 0
	BEGIN
		SET @CurGold = @CurGold + @GOLD
		UPDATE Tbl_Player  SET Gold = @CurGold WHERE PlayerID = @PLAYERID
	END
	ELSE
		SET @Err = 1

	IF @Err <> 0
		ROLLBACK TRANSACTION UpdatePlayerGold
	ELSE
		COMMIT TRANSACTION UpdatePlayerGold

	SELECT @Err
END




