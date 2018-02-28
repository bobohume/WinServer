CREATE PROCEDURE [dbo].[USP_PLAYERRENAME]
@Name VARCHAR (32), 
@AccountID INT,
@PlayerID  INT
AS
BEGIN
	SET NOCOUNT ON
	DECLARE @Result CHAR(4)
	SET @Result = '0001'
	
	IF NOT EXISTS(SELECT 1 FROM TBL_PLAYER 
		WHERE PlayerName = @Name AND [Delete] = 0)
	BEGIN
		UPDATE TBL_PLAYER SET PlayerName = @Name 
			WHERE AccountID = @AccountID AND PlayerID = @PlayerID
		IF @@ROWCOUNT = 0
			SET @Result = '0002'
		ELSE
			SET @Result = '0000'
	END

	SELECT @Result
END