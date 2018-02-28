-- =============================================
-- Author:		<LivenHotch>
-- Create date: <2010-05-27>
-- Description:	<内购不在线加钱处理>
-- modifyer:    <Daniel>
-- =============================================
CREATE PROCEDURE [dbo].[Sp_UpdatePlayerDrawGold]
@PLAYERID		INT,
@GOLD			INT
AS
BEGIN
	SET NOCOUNT ON
	SET XACT_ABORT ON

	DECLARE @CurGold INT
	SET @CurGold = 0
	DECLARE @Err INT
	SET @Err = 0

	BEGIN TRANSACTION UpdatePlayerDrawGold
	SELECT @CurGold = DrawGold From Tbl_Player WHERE PlayerID = @PLAYERID
	
	IF @@ROWCOUNT <> 0
	BEGIN 
		SET @CurGold = @CurGold + @GOLD
		UPDATE Tbl_Player SET DrawGold = @CurGold WHERE PlayerID = @PLAYERID
	END
	ELSE
		SET @Err = -1

	IF @Err <> 0
		ROLLBACK TRANSACTION UpdatePlayerDrawGold
	ELSE
		COMMIT   TRANSACTION UpdatePlayerDrawGold

	SELECT @Err
END




